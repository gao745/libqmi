/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * libqmi-glib -- GLib/GIO based library to control QMI devices
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 * Copyright (C) 2012 Lanedo GmbH
 * Copyright (C) 2012-2017 Aleksander Morgado <aleksander@aleksander.es>
 */

#include <config.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "qmi-device.h"
#include "qmi-message.h"
#include "qmi-file.h"
#include "qmi-endpoint.h"
#include "qmi-endpoint-mbim.h"
#include "qmi-endpoint-qmux.h"
#include "qmi-ctl.h"
#include "qmi-dms.h"
#include "qmi-wds.h"
#include "qmi-nas.h"
#include "qmi-wms.h"
#include "qmi-pdc.h"
#include "qmi-pds.h"
#include "qmi-pbm.h"
#include "qmi-uim.h"
#include "qmi-oma.h"
#include "qmi-wda.h"
#include "qmi-voice.h"
#include "qmi-loc.h"
#include "qmi-qos.h"
#include "qmi-gas.h"
#include "qmi-utils.h"
#include "qmi-error-types.h"
#include "qmi-enum-types.h"
#include "qmi-proxy.h"

static void async_initable_iface_init (GAsyncInitableIface *iface);

G_DEFINE_TYPE_EXTENDED (QmiDevice, qmi_device, G_TYPE_OBJECT, 0,
                        G_IMPLEMENT_INTERFACE (G_TYPE_ASYNC_INITABLE, async_initable_iface_init))

enum {
    PROP_0,
    PROP_FILE,
    PROP_NO_FILE_CHECK,
    PROP_PROXY_PATH,
    PROP_WWAN_IFACE,
    PROP_LAST
};

enum {
    SIGNAL_INDICATION,
    SIGNAL_REMOVED,
    SIGNAL_LAST
};

static GParamSpec *properties[PROP_LAST];
static guint       signals   [SIGNAL_LAST] = { 0 };

struct _QmiDevicePrivate {
    /* File */
    QmiFile *file;
    gboolean no_file_check;

    /* WWAN interface */
    gboolean no_wwan_check;
    gchar *wwan_iface;

    /* Implicit CTL client */
    QmiClientCtl *client_ctl;
    guint sync_indication_id;

    /* Supported services */
    GArray *supported_services;

    /* Lower-level transport */
    QmiEndpoint *endpoint;
    guint endpoint_new_data_id;
    guint endpoint_hangup_id;

    /* Support for qmi-proxy */
    gchar *proxy_path;

    /* HT to keep track of ongoing transactions */
    GHashTable *transactions;

    /* HT of clients that want to get indications */
    GHashTable *registered_clients;
};

/*****************************************************************************/
/* Message transactions (private) */

typedef struct {
    QmiDevice *self;
    gpointer   key;
} TransactionWaitContext;

typedef struct {
    QmiMessage             *message;
    QmiMessageContext      *message_context;
    GSimpleAsyncResult     *result;
    GSource                *timeout_source;
    GCancellable           *cancellable;
    gulong                  cancellable_id;
    TransactionWaitContext *wait_ctx;

    /* abortable support */
    GError                                   *abort_error;
    GCancellable                             *abort_cancellable;
    QmiDeviceCommandAbortableBuildRequestFn   abort_build_request_fn;
    QmiDeviceCommandAbortableParseResponseFn  abort_parse_response_fn;
    gpointer                                  abort_user_data;
    GDestroyNotify                            abort_user_data_free;
} Transaction;

static Transaction *
transaction_new (QmiDevice           *self,
                 QmiMessage          *message,
                 QmiMessageContext   *message_context,
                 GCancellable        *cancellable,
                 GAsyncReadyCallback  callback,
                 gpointer             user_data)
{
    Transaction *tr;

    tr = g_slice_new0 (Transaction);
    tr->message = qmi_message_ref (message);
    tr->message_context = (message_context ? qmi_message_context_ref (message_context) : NULL);
    tr->result = g_simple_async_result_new (G_OBJECT (self),
                                            callback,
                                            user_data,
                                            transaction_new);
    if (cancellable)
        tr->cancellable = g_object_ref (cancellable);

    return tr;
}

static void
transaction_complete_and_free (Transaction  *tr,
                               QmiMessage   *reply,
                               const GError *error)
{
    g_assert (reply != NULL || error != NULL);

    /* always set result first, as we may be using one of the GErrors
     * stored in the Transaction as result itself */
    if (reply) {
        /* if we got a valid response, we can cancel any ongoing abort
         * operation for this request */
        if (tr->abort_cancellable) {
            g_debug ("transaction 0x%x completed with a response: cancelling the abort operation",
                     qmi_message_get_transaction_id (tr->message));
            g_cancellable_cancel (tr->abort_cancellable);
        }
        g_simple_async_result_set_op_res_gpointer (tr->result,
                                                   qmi_message_ref (reply),
                                                   (GDestroyNotify)qmi_message_unref);
    } else if (error)
        g_simple_async_result_set_from_error (tr->result, error);
    else
        g_assert_not_reached ();

    if (tr->timeout_source)
        g_source_destroy (tr->timeout_source);

    if (tr->cancellable) {
        if (tr->cancellable_id)
            g_cancellable_disconnect (tr->cancellable, tr->cancellable_id);
        g_object_unref (tr->cancellable);
    }

    if (tr->wait_ctx)
        g_slice_free (TransactionWaitContext, tr->wait_ctx);

    if (tr->abort_error)
        g_error_free (tr->abort_error);

    if (tr->abort_cancellable)
        g_object_unref (tr->abort_cancellable);

    if (tr->abort_user_data && tr->abort_user_data_free)
        tr->abort_user_data_free (tr->abort_user_data);

    g_simple_async_result_complete_in_idle (tr->result);
    g_object_unref (tr->result);
    if (tr->message_context)
        qmi_message_context_unref (tr->message_context);
    qmi_message_unref (tr->message);
    g_slice_free (Transaction, tr);
}

static inline gpointer
build_transaction_key (QmiMessage *message)
{
    gpointer key;
    guint8 service;
    guint8 client_id;
    guint16 transaction_id;

    service = (guint8)qmi_message_get_service (message);
    client_id = qmi_message_get_client_id (message);
    transaction_id = qmi_message_get_transaction_id (message);

    /* We're putting a 32 bit value into a gpointer */
    key = GUINT_TO_POINTER ((((service << 8) | client_id) << 16) | transaction_id);

    return key;
}

static Transaction *
device_peek_transaction (QmiDevice *self,
                         gconstpointer key)
{
    return g_hash_table_lookup (self->priv->transactions, key);
}

static Transaction *
device_release_transaction (QmiDevice *self,
                            gconstpointer key)
{
    Transaction *tr = NULL;

    /* If found, remove it from the HT */
    tr = device_peek_transaction (self, key);
    if (tr)
        g_hash_table_remove (self->priv->transactions, key);

    return tr;
}

static void
transaction_abort_ready (QmiDevice    *self,
                         GAsyncResult *res,
                         gpointer      key)
{
    Transaction *tr;
    GError      *error = NULL;
    QmiMessage  *abort_response;

    /* Always try to remove from the HT at this point. Note that the transaction
     * pointer may be NULL already, e.g. if the abort was cancelled. In this case
     * we totally ignore the result of the abort operation. */
    tr = device_release_transaction (self, key);
    if (!tr) {
        g_debug ("not processing abort response, operation has already been completed");
        return;
    }

    g_assert (tr->abort_parse_response_fn);

    abort_response = qmi_device_command_full_finish (self, res, &error);
    if (!abort_response ||
        !tr->abort_parse_response_fn (self,
                                      abort_response,
                                      tr->abort_user_data,
                                      &error)) {
        GError *built_error;

        g_debug ("abort operation failed: %s", error->message);

        /* We don't want to return any kind of error, because what failed here
         * is the abort operation for the user request, so always return
         * QMI_CORE_ERROR_FAILED and provide the description on the error
         * in the message. */
        built_error = g_error_new (QMI_CORE_ERROR, QMI_CORE_ERROR_FAILED,
                                   "operation failed and couldn't be aborted: %s",
                                   error->message);
        g_error_free (error);

        transaction_complete_and_free (tr, NULL, built_error);
        g_error_free (built_error);
    } else {
        /* aborted successfully */
        g_debug ("operation aborted successfully");
        g_assert (tr->abort_error);
        transaction_complete_and_free (tr, NULL, tr->abort_error);
    }

    if (abort_response)
        qmi_message_unref (abort_response);
}

static void
transaction_abort (QmiDevice   *self,
                   Transaction *tr,
                   GError      *abort_error_take)
{
    QmiMessage *abort_request;
    GError     *error = NULL;
    guint16     transaction_id;

    transaction_id = qmi_message_get_transaction_id (tr->message);

    /* If the command is not abortable, we'll return the error right away
     * to the user. */
    if (!__qmi_message_is_abortable (tr->message, tr->message_context)) {
        g_debug ("transaction 0x%x aborted, but message is not abortable", transaction_id);
        device_release_transaction (self, tr->wait_ctx->key);
        transaction_complete_and_free (tr, NULL, abort_error_take);
        g_error_free (abort_error_take);
        return;
    }

    /* if the command is abortable but the user didn't use qmi_device_command_abortable(),
     * then return the error right away anyway */
    if (!tr->abort_build_request_fn || !tr->abort_parse_response_fn) {
        g_debug ("transaction 0x%x aborted, but no way to build abort request", transaction_id);
        device_release_transaction (self, tr->wait_ctx->key);
        transaction_complete_and_free (tr, NULL, abort_error_take);
        g_error_free (abort_error_take);
        return;
    }

    g_debug ("transaction 0x%x aborted, building abort request...", transaction_id);

    /* Try to build abort request */
    abort_request = tr->abort_build_request_fn (self,
                                                tr->message,
                                                tr->abort_user_data,
                                                &error);
    if (!abort_request) {
        /* complete the transaction with the error we got while building the
         * abort request */
        g_debug ("transaction 0x%x aborted, but building abort request failed", transaction_id);
        device_release_transaction (self, tr->wait_ctx->key);
        transaction_complete_and_free (tr, NULL, error);
        g_error_free (error);
        return;
    }

    /* If command is abortable, let's abort the operation in the
     * device. We'll store the specific abort error to use once the abort
     * operation has been acknowledged by the device. */
    tr->abort_error = abort_error_take;
    tr->abort_cancellable = g_cancellable_new ();

    qmi_device_command_full (self,
                             abort_request,
                             NULL,
                             30,
                             tr->abort_cancellable,
                             (GAsyncReadyCallback) transaction_abort_ready,
                             tr->wait_ctx->key);

    qmi_message_unref (abort_request);
}

static gboolean
transaction_timed_out (TransactionWaitContext *ctx)
{
    Transaction *tr;
    GError *error = NULL;

    /* A timed out transaction is always tracked */
    tr = device_peek_transaction (ctx->self, ctx->key);
    g_assert (tr);

    tr->timeout_source = NULL;

    error = g_error_new (QMI_CORE_ERROR, QMI_CORE_ERROR_TIMEOUT, "Transaction timed out");
    transaction_abort (ctx->self, tr, error);

    return G_SOURCE_REMOVE;
}

static void
transaction_cancelled (GCancellable *cancellable,
                       TransactionWaitContext *ctx)
{
    Transaction *tr;
    GError *error = NULL;

    tr = device_peek_transaction (ctx->self, ctx->key);

    /* The transaction may have already been cancelled before we stored it in
     * the tracking table, which means the command was NOT sent to the device
     * and we can safely ignore the cancellation request. */
    if (!tr)
        return;

    tr->cancellable_id = 0;

    error = g_error_new (QMI_PROTOCOL_ERROR, QMI_PROTOCOL_ERROR_ABORTED, "Transaction aborted");
    transaction_abort (ctx->self, tr, error);
}

static gboolean
device_store_transaction (QmiDevice *self,
                          Transaction *tr,
                          guint timeout,
                          GError **error)
{
    gpointer     key;
    Transaction *existing;

    key = build_transaction_key (tr->message);

    /* Setup the timeout and cancellation */

    tr->wait_ctx = g_slice_new (TransactionWaitContext);
    tr->wait_ctx->self = self;
    tr->wait_ctx->key = key; /* valid as long as the transaction is in the HT */

    /* Timeout is optional (e.g. disabled when MBIM is used) */
    if (timeout > 0) {
        tr->timeout_source = g_timeout_source_new_seconds (timeout);
        g_source_set_callback (tr->timeout_source, (GSourceFunc)transaction_timed_out, tr->wait_ctx, NULL);
        g_source_attach (tr->timeout_source, g_main_context_get_thread_default ());
        g_source_unref (tr->timeout_source);
    }

    if (tr->cancellable) {
        /* Note: transaction_cancelled() will also be called directly if the
         * cancellable is already cancelled */
        tr->cancellable_id = g_cancellable_connect (tr->cancellable,
                                                    (GCallback)transaction_cancelled,
                                                    tr->wait_ctx,
                                                    NULL);
        if (!tr->cancellable_id) {
            g_set_error (error,
                         QMI_PROTOCOL_ERROR,
                         QMI_PROTOCOL_ERROR_ABORTED,
                         "Request is already cancelled");
            return FALSE;
        }
    }

    /* If we have already a transaction with the same ID complete the existing
     * one with an error before the new one is added, or we'll end up with
     * dangling timeouts and cancellation handlers that may be fired off later
     * on. */
    existing = device_release_transaction (self, key);
    if (existing) {
        GError *inner_error;

        /* Complete transaction with an abort error */
        inner_error = g_error_new (QMI_PROTOCOL_ERROR,
                                   QMI_PROTOCOL_ERROR_ABORTED,
                                   "Transaction overwritten");
        transaction_complete_and_free (existing, NULL, inner_error);
        g_error_free (inner_error);
    }

    /* Keep in the HT */
    g_hash_table_insert (self->priv->transactions, key, tr);

    return TRUE;
}

static Transaction *
device_match_transaction (QmiDevice *self,
                          QmiMessage *message)
{
    /* msg can be either the original message or the response */
    return device_release_transaction (self, build_transaction_key (message));
}

/*****************************************************************************/
/* Version info request */

GArray *
qmi_device_get_service_version_info_finish (QmiDevice *self,
                                            GAsyncResult *res,
                                            GError **error)
{
    return g_task_propagate_pointer (G_TASK (res), error);
}

static void
version_info_ready (QmiClientCtl *client_ctl,
                    GAsyncResult *res,
                    GTask *task)
{
    GArray *service_list = NULL;
    GArray *out;
    QmiMessageCtlGetVersionInfoOutput *output;
    GError *error = NULL;
    guint i;

    /* Check result of the async operation */
    output = qmi_client_ctl_get_version_info_finish (client_ctl, res, &error);
    if (!output) {
        g_task_return_error (task, error);
        g_object_unref (task);
        return;
    }

    /* Check result of the QMI operation */
    if (!qmi_message_ctl_get_version_info_output_get_result (output, &error)) {
        qmi_message_ctl_get_version_info_output_unref (output);
        g_task_return_error (task, error);
        g_object_unref (task);
        return;
    }

    /* QMI operation succeeded, we can now get the outputs */
    qmi_message_ctl_get_version_info_output_get_service_list (output, &service_list, NULL);
    out = g_array_sized_new (FALSE, FALSE, sizeof (QmiDeviceServiceVersionInfo), service_list->len);
    for (i = 0; i < service_list->len; i++) {
        QmiMessageCtlGetVersionInfoOutputServiceListService *info;
        QmiDeviceServiceVersionInfo outinfo;

        info = &g_array_index (service_list,
                               QmiMessageCtlGetVersionInfoOutputServiceListService,
                               i);
        outinfo.service = info->service;
        outinfo.major_version = info->major_version;
        outinfo.minor_version = info->minor_version;
        g_array_append_val (out, outinfo);
    }

    qmi_message_ctl_get_version_info_output_unref (output);
    g_task_return_pointer (task, out, (GDestroyNotify)g_array_unref);
    g_object_unref (task);
}

void
qmi_device_get_service_version_info (QmiDevice *self,
                                     guint timeout,
                                     GCancellable *cancellable,
                                     GAsyncReadyCallback callback,
                                     gpointer user_data)
{
    qmi_client_ctl_get_version_info (
        self->priv->client_ctl,
        NULL,
        timeout,
        cancellable,
        (GAsyncReadyCallback)version_info_ready,
        g_task_new (self, cancellable, callback, user_data));
}

/*****************************************************************************/
/* Version info checks (private) */

static const QmiMessageCtlGetVersionInfoOutputServiceListService *
find_service_version_info (QmiDevice *self,
                           QmiService service)
{
    guint i;

    if (!self->priv->supported_services)
        return NULL;

    for (i = 0; i < self->priv->supported_services->len; i++) {
        const QmiMessageCtlGetVersionInfoOutputServiceListService *info;

        info = &g_array_index (self->priv->supported_services,
                               QmiMessageCtlGetVersionInfoOutputServiceListService,
                               i);

        if (service == info->service)
            return info;
    }

    return NULL;
}

static gboolean
check_service_supported (QmiDevice *self,
                         QmiService service)
{
    /* If we didn't check supported services, just assume it is supported */
    if (!self->priv->supported_services) {
        g_debug ("[%s] Assuming service '%s' is supported...",
                 qmi_file_get_path_display (self->priv->file),
                 qmi_service_get_string (service));
        return TRUE;
    }

    return !!find_service_version_info (self, service);
}

static gboolean
check_message_supported (QmiDevice *self,
                         QmiMessage *message,
                         GError **error)
{
    const QmiMessageCtlGetVersionInfoOutputServiceListService *info;
    guint message_major = 0;
    guint message_minor = 0;
    guint device_major = 0;
    guint device_minor = 0;

    /* If we didn't check supported services, just assume it is supported */
    if (!self->priv->supported_services)
        return TRUE;

    /* For CTL, we assume all are supported */
    if (qmi_message_get_service (message) == QMI_SERVICE_CTL)
        return TRUE;

    /* If we cannot get in which version this message was introduced, we'll just
     * assume it's supported */
    if (!qmi_message_get_version_introduced_full (message, NULL, &message_major, &message_minor))
        return TRUE;

    /* Get version info. It MUST exist because we allowed creating a client
     * of this service type */
    info = find_service_version_info (self, qmi_message_get_service (message));
    g_assert (info != NULL);
    g_assert (info->service == qmi_message_get_service (message));
    device_major = info->major_version;
    device_minor = info->minor_version;

    /* Some device firmware versions (Quectel EC21) lie about their supported
     * DMS version, so assume a reasonable DMS version if the WDS version is
     * high enough */
    if (info->service == QMI_SERVICE_DMS && device_major == 1 && device_minor == 0) {
        const QmiMessageCtlGetVersionInfoOutputServiceListService *wds;

        wds = find_service_version_info (self, QMI_SERVICE_WDS);
        g_assert (wds != NULL);
        if (wds->major_version >= 1 && wds->minor_version >= 9) {
            device_major = 1;
            device_minor = 3;
        }
    }

    /* If the version of the message is greater than the version of the service,
     * report unsupported */
    if (message_major > device_major ||
        (message_major == device_major &&
         message_minor > device_minor)) {
        g_set_error (error,
                     QMI_CORE_ERROR,
                     QMI_CORE_ERROR_UNSUPPORTED,
                     "QMI service '%s' version '%u.%u' required, got version '%u.%u'",
                     qmi_service_get_string (qmi_message_get_service (message)),
                     message_major, message_minor,
                     info->major_version,
                     info->minor_version);
        return FALSE;
    }

    /* Supported! */
    return TRUE;
}

/*****************************************************************************/

GFile *
qmi_device_get_file (QmiDevice *self)
{
    g_return_val_if_fail (QMI_IS_DEVICE (self), NULL);

    return qmi_file_get_file (self->priv->file);
}

GFile *
qmi_device_peek_file (QmiDevice *self)
{
    g_return_val_if_fail (QMI_IS_DEVICE (self), NULL);

    return qmi_file_peek_file (self->priv->file);
}

const gchar *
qmi_device_get_path (QmiDevice *self)
{
    g_return_val_if_fail (QMI_IS_DEVICE (self), NULL);

    return qmi_file_get_path (self->priv->file);
}

const gchar *
qmi_device_get_path_display (QmiDevice *self)
{
    g_return_val_if_fail (QMI_IS_DEVICE (self), NULL);

    return qmi_file_get_path_display (self->priv->file);
}

gboolean
qmi_device_is_open (QmiDevice *self)
{
    g_return_val_if_fail (QMI_IS_DEVICE (self), FALSE);

    return !!self->priv->endpoint && qmi_endpoint_is_open (self->priv->endpoint);
}

/*****************************************************************************/
/* WWAN iface name
 * Always reload from scratch, to handle possible net interface renames  */

static void
reload_wwan_iface_name (QmiDevice *self)
{
    gchar *cdc_wdm_device_name;
    static const gchar *driver_names[] = { "usbmisc", "usb" };
    GError *error = NULL;
    guint i;

    /* Early cleanup */
    g_free (self->priv->wwan_iface);
    self->priv->wwan_iface = NULL;

    cdc_wdm_device_name = __qmi_utils_get_devname (qmi_file_get_path (self->priv->file), &error);
    if (!cdc_wdm_device_name) {
        g_warning ("[%s] invalid path for cdc-wdm control port: %s",
                   qmi_file_get_path_display (self->priv->file),
                   error->message);
        g_error_free (error);
        return;
    }

    for (i = 0; i < G_N_ELEMENTS (driver_names) && !self->priv->wwan_iface; i++) {
        gchar *sysfs_path;
        GFile *sysfs_file;
        GFileEnumerator *enumerator;

        sysfs_path = g_strdup_printf ("/sys/class/%s/%s/device/net/", driver_names[i], cdc_wdm_device_name);
        sysfs_file = g_file_new_for_path (sysfs_path);
        enumerator = g_file_enumerate_children (sysfs_file,
                                                G_FILE_ATTRIBUTE_STANDARD_NAME,
                                                G_FILE_QUERY_INFO_NONE,
                                                NULL,
                                                &error);
        if (!enumerator) {
            g_debug ("[%s] cannot enumerate files at path '%s': %s",
                     qmi_file_get_path_display (self->priv->file),
                     sysfs_path,
                     error->message);
            g_error_free (error);
        } else {
            GFileInfo *file_info;

            /* Ignore errors when enumerating */
            while ((file_info = g_file_enumerator_next_file (enumerator, NULL, NULL)) != NULL) {
                const gchar *name;

                name = g_file_info_get_name (file_info);
                if (name) {
                    /* We only expect ONE file in the sysfs directory corresponding
                     * to this control port, if more found for any reason, warn about it */
                    if (self->priv->wwan_iface)
                        g_warning ("[%s] invalid additional wwan iface found: %s",
                               qmi_file_get_path_display (self->priv->file), name);
                    else
                        self->priv->wwan_iface = g_strdup (name);
                }
                g_object_unref (file_info);
            }

            g_object_unref (enumerator);
        }

        g_free (sysfs_path);
        g_object_unref (sysfs_file);
    }
    g_free (cdc_wdm_device_name);
    if (!self->priv->wwan_iface)
        g_warning ("[%s] wwan iface not found", qmi_file_get_path_display (self->priv->file));
}

const gchar *
qmi_device_get_wwan_iface (QmiDevice *self)
{
    g_return_val_if_fail (QMI_IS_DEVICE (self), NULL);

    reload_wwan_iface_name (self);
    return self->priv->wwan_iface;
}

/*****************************************************************************/
/* Expected data format */

static gboolean
get_expected_data_format (QmiDevice *self,
                          const gchar *sysfs_path,
                          GError **error)
{
    QmiDeviceExpectedDataFormat expected = QMI_DEVICE_EXPECTED_DATA_FORMAT_UNKNOWN;
    gchar value = '\0';
    FILE *f;

    g_debug ("[%s] Reading expected data format from: %s",
             qmi_file_get_path_display (self->priv->file),
             sysfs_path);

    if (!(f = fopen (sysfs_path, "r"))) {
        g_set_error (error, G_IO_ERROR, g_io_error_from_errno (errno),
                     "Failed to open file '%s': %s",
                     sysfs_path, g_strerror (errno));
        goto out;
    }

    if (fread (&value, 1, 1, f) != 1) {
        g_set_error (error, G_IO_ERROR, g_io_error_from_errno (errno),
                     "Failed to read from file '%s': %s",
                     sysfs_path, g_strerror (errno));
        goto out;
    }

    if (value == 'Y')
        expected = QMI_DEVICE_EXPECTED_DATA_FORMAT_RAW_IP;
    else if (value == 'N')
        expected = QMI_DEVICE_EXPECTED_DATA_FORMAT_802_3;
    else
        g_set_error (error, QMI_CORE_ERROR, QMI_CORE_ERROR_FAILED,
                     "Unexpected sysfs file contents");

 out:
    g_prefix_error (error, "Expected data format not retrieved properly: ");
    if (f)
        fclose (f);
    return expected;
}

static gboolean
set_expected_data_format (QmiDevice *self,
                          const gchar *sysfs_path,
                          QmiDeviceExpectedDataFormat requested,
                          GError **error)
{
    gboolean status = FALSE;
    gchar value;
    FILE *f;

    g_debug ("[%s] Writing expected data format to: %s",
             qmi_file_get_path_display (self->priv->file),
             sysfs_path);

    if (requested == QMI_DEVICE_EXPECTED_DATA_FORMAT_RAW_IP)
        value = 'Y';
    else if (requested == QMI_DEVICE_EXPECTED_DATA_FORMAT_802_3)
        value = 'N';
    else
        g_assert_not_reached ();

    if (!(f = fopen (sysfs_path, "w"))) {
        g_set_error (error, G_IO_ERROR, g_io_error_from_errno (errno),
                     "Failed to open file '%s' for R/W: %s",
                     sysfs_path, g_strerror (errno));
        goto out;
    }

    if (fwrite (&value, 1, 1, f) != 1) {
        g_set_error (error, G_IO_ERROR, g_io_error_from_errno (errno),
                     "Failed to write to file '%s': %s",
                     sysfs_path, g_strerror (errno));
        goto out;
    }

    status = TRUE;

 out:
    g_prefix_error (error, "Expected data format not updated properly: ");
    if (f)
        fclose (f);
    return status;
}

static QmiDeviceExpectedDataFormat
common_get_set_expected_data_format (QmiDevice *self,
                                     QmiDeviceExpectedDataFormat requested,
                                     GError **error)
{
    gchar *sysfs_path = NULL;
    QmiDeviceExpectedDataFormat expected = QMI_DEVICE_EXPECTED_DATA_FORMAT_UNKNOWN;
    gboolean readonly;

    readonly = (requested == QMI_DEVICE_EXPECTED_DATA_FORMAT_UNKNOWN);

    /* Make sure we load the WWAN iface name */
    reload_wwan_iface_name (self);
    if (!self->priv->wwan_iface) {
        g_set_error (error, QMI_CORE_ERROR, QMI_CORE_ERROR_FAILED,
                     "Unknown wwan iface");
        goto out;
    }

    /* Build sysfs file path and open it */
    sysfs_path = g_strdup_printf ("/sys/class/net/%s/qmi/raw_ip", self->priv->wwan_iface);

    /* Set operation? */
    if (!readonly && !set_expected_data_format (self, sysfs_path, requested, error))
        goto out;

    /* Get/Set operations */
    if ((expected = get_expected_data_format (self, sysfs_path, error)) == QMI_DEVICE_EXPECTED_DATA_FORMAT_UNKNOWN)
        goto out;

    /* If we requested an update but we didn't read that value, report an error */
    if (!readonly && (requested != expected)) {
        g_set_error (error, QMI_CORE_ERROR, QMI_CORE_ERROR_FAILED,
                     "Expected data format not updated properly to '%s': got '%s' instead",
                     qmi_device_expected_data_format_get_string (requested),
                     qmi_device_expected_data_format_get_string (expected));
        expected = QMI_DEVICE_EXPECTED_DATA_FORMAT_UNKNOWN;
    }

 out:
    g_free (sysfs_path);
    return expected;
}

QmiDeviceExpectedDataFormat
qmi_device_get_expected_data_format (QmiDevice  *self,
                                     GError    **error)
{
    g_return_val_if_fail (QMI_IS_DEVICE (self), QMI_DEVICE_EXPECTED_DATA_FORMAT_UNKNOWN);

    return common_get_set_expected_data_format (self, QMI_DEVICE_EXPECTED_DATA_FORMAT_UNKNOWN, error);
}

gboolean
qmi_device_set_expected_data_format (QmiDevice *self,
                                     QmiDeviceExpectedDataFormat format,
                                     GError **error)
{
    g_return_val_if_fail (QMI_IS_DEVICE (self), FALSE);

    return (common_get_set_expected_data_format (self, format, error) != QMI_DEVICE_EXPECTED_DATA_FORMAT_UNKNOWN);
}

/*****************************************************************************/
/* Register/Unregister clients that want to receive indications */

static gpointer
build_registered_client_key (guint8 cid,
                             QmiService service)
{
    return GUINT_TO_POINTER (((guint8)service << 8) | cid);
}

static gboolean
register_client (QmiDevice *self,
                 QmiClient *client,
                 GError **error)
{
    gpointer key;

    key = build_registered_client_key (qmi_client_get_cid (client),
                                       qmi_client_get_service (client));
    /* Only add the new client if not already registered one with the same CID
     * for the same service */
    if (g_hash_table_lookup (self->priv->registered_clients, key)) {
        g_set_error (error,
                     QMI_CORE_ERROR,
                     QMI_CORE_ERROR_FAILED,
                     "A client with CID '%u' and service '%s' is already registered",
                     qmi_client_get_cid (client),
                     qmi_service_get_string (qmi_client_get_service (client)));
        return FALSE;
    }

    g_hash_table_insert (self->priv->registered_clients,
                         key,
                         g_object_ref (client));
    return TRUE;
}

static void
unregister_client (QmiDevice *self,
                   QmiClient *client)
{
    g_hash_table_remove (self->priv->registered_clients,
                         build_registered_client_key (qmi_client_get_cid (client),
                                                      qmi_client_get_service (client)));
}

/*****************************************************************************/
/* Allocate new client */

typedef struct {
    QmiService service;
    GType client_type;
    guint8 cid;
} AllocateClientContext;

static void
allocate_client_context_free (AllocateClientContext *ctx)
{
    g_slice_free (AllocateClientContext, ctx);
}

QmiClient *
qmi_device_allocate_client_finish (QmiDevice *self,
                                   GAsyncResult *res,
                                   GError **error)
{
    return g_task_propagate_pointer (G_TASK (res), error);
}

static void
build_client_object (GTask *task)
{
    QmiDevice *self;
    AllocateClientContext *ctx;
    gchar *version_string = NULL;
    QmiClient *client;
    GError *error = NULL;
    const QmiMessageCtlGetVersionInfoOutputServiceListService *version_info;

    self = g_task_get_source_object (task);
    ctx = g_task_get_task_data (task);

    /* We now have a proper CID for the client, we should be able to create it
     * right away */
    client = g_object_new (ctx->client_type,
                           QMI_CLIENT_DEVICE,  self,
                           QMI_CLIENT_SERVICE, ctx->service,
                           QMI_CLIENT_CID,     ctx->cid,
                           NULL);

    /* Add version info to the client if it was retrieved */
    version_info = find_service_version_info (self, ctx->service);
    if (version_info)
        g_object_set (client,
                      QMI_CLIENT_VERSION_MAJOR, version_info->major_version,
                      QMI_CLIENT_VERSION_MINOR, version_info->minor_version,
                      NULL);

    /* Register the client to get indications */
    if (!register_client (self, client, &error)) {
        g_prefix_error (&error,
                        "Cannot register new client with CID '%u' and service '%s'",
                        ctx->cid,
                        qmi_service_get_string (ctx->service));
        g_task_return_error (task, error);
        g_object_unref (task);
        g_object_unref (client);
        return;
    }

    /* Build version string for the logging */
    if (self->priv->supported_services) {
        const QmiMessageCtlGetVersionInfoOutputServiceListService *info;

        info = find_service_version_info (self, ctx->service);
        if (info)
            version_string = g_strdup_printf ("%u.%u", info->major_version, info->minor_version);
    }

    g_debug ("[%s] Registered '%s' (version %s) client with ID '%u'",
             qmi_file_get_path_display (self->priv->file),
             qmi_service_get_string (ctx->service),
             version_string ? version_string : "unknown",
             ctx->cid);

    g_free (version_string);

    /* Client created and registered, complete successfully */
    g_task_return_pointer (task, client, g_object_unref);
    g_object_unref (task);
}

static void
allocate_cid_ready (QmiClientCtl *client_ctl,
                    GAsyncResult *res,
                    GTask *task)
{
    QmiMessageCtlAllocateCidOutput *output;
    QmiService service;
    guint8 cid;
    GError *error = NULL;
    AllocateClientContext *ctx;

    /* Check result of the async operation */
    output = qmi_client_ctl_allocate_cid_finish (client_ctl, res, &error);
    if (!output) {
        g_prefix_error (&error, "CID allocation failed in the CTL client: ");
        g_task_return_error (task, error);
        g_object_unref (task);
        return;
    }

    /* Check result of the QMI operation */
    if (!qmi_message_ctl_allocate_cid_output_get_result (output, &error)) {
        g_task_return_error (task, error);
        g_object_unref (task);
        qmi_message_ctl_allocate_cid_output_unref (output);
        return;
    }

    /* Allocation info is mandatory when result is success */
    g_assert (qmi_message_ctl_allocate_cid_output_get_allocation_info (output, &service, &cid, NULL));

    ctx = g_task_get_task_data (task);

    if (service != ctx->service) {
        g_task_return_new_error (
            task,
            QMI_CORE_ERROR,
            QMI_CORE_ERROR_FAILED,
            "CID allocation failed in the CTL client: "
            "Service mismatch (requested '%s', got '%s')",
            qmi_service_get_string (ctx->service),
            qmi_service_get_string (service));
        g_object_unref (task);
        qmi_message_ctl_allocate_cid_output_unref (output);
        return;
    }

    ctx->cid = cid;
    build_client_object (task);
    qmi_message_ctl_allocate_cid_output_unref (output);
}

void
qmi_device_allocate_client (QmiDevice *self,
                            QmiService service,
                            guint8 cid,
                            guint timeout,
                            GCancellable *cancellable,
                            GAsyncReadyCallback callback,
                            gpointer user_data)
{
    AllocateClientContext *ctx;
    GTask *task;

    g_return_if_fail (QMI_IS_DEVICE (self));
    g_return_if_fail (service != QMI_SERVICE_UNKNOWN);

    ctx = g_slice_new0 (AllocateClientContext);
    ctx->service = service;

    task = g_task_new (self, cancellable, callback, user_data);
    g_task_set_task_data (task,
                          ctx,
                          (GDestroyNotify)allocate_client_context_free);

    /* Check if the requested service is supported by the device */
    if (!check_service_supported (self, service)) {
        g_task_return_new_error (task,
                                 QMI_CORE_ERROR,
                                 QMI_CORE_ERROR_UNSUPPORTED,
                                 "Service '%s' not supported by the device",
                                 qmi_service_get_string (service));
        g_object_unref (task);
        return;
    }

    switch (service) {
    case QMI_SERVICE_CTL:
        g_task_return_new_error (task,
                                 QMI_CORE_ERROR,
                                 QMI_CORE_ERROR_INVALID_ARGS,
                                 "Cannot create additional clients for the CTL service");
        g_object_unref (task);
        return;

    case QMI_SERVICE_DMS:
        ctx->client_type = QMI_TYPE_CLIENT_DMS;
        break;

    case QMI_SERVICE_WDS:
        ctx->client_type = QMI_TYPE_CLIENT_WDS;
        break;

    case QMI_SERVICE_NAS:
        ctx->client_type = QMI_TYPE_CLIENT_NAS;
        break;

    case QMI_SERVICE_WMS:
        ctx->client_type = QMI_TYPE_CLIENT_WMS;
        break;

    case QMI_SERVICE_PDS:
        ctx->client_type = QMI_TYPE_CLIENT_PDS;
        break;

    case QMI_SERVICE_PDC:
        ctx->client_type = QMI_TYPE_CLIENT_PDC;
        break;

    case QMI_SERVICE_PBM:
        ctx->client_type = QMI_TYPE_CLIENT_PBM;
        break;

    case QMI_SERVICE_UIM:
        ctx->client_type = QMI_TYPE_CLIENT_UIM;
        break;

    case QMI_SERVICE_OMA:
        ctx->client_type = QMI_TYPE_CLIENT_OMA;
        break;

    case QMI_SERVICE_GAS:
        ctx->client_type = QMI_TYPE_CLIENT_GAS;
        break;

    case QMI_SERVICE_WDA:
        ctx->client_type = QMI_TYPE_CLIENT_WDA;
        break;

    case QMI_SERVICE_VOICE:
        ctx->client_type = QMI_TYPE_CLIENT_VOICE;
        break;

    case QMI_SERVICE_LOC:
        ctx->client_type = QMI_TYPE_CLIENT_LOC;
        break;

    case QMI_SERVICE_QOS:
        ctx->client_type = QMI_TYPE_CLIENT_QOS;
        break;

    default:
        g_task_return_new_error (task,
                                 QMI_CORE_ERROR,
                                 QMI_CORE_ERROR_INVALID_ARGS,
                                 "Clients for service '%s' not yet supported",
                                 qmi_service_get_string (service));
        g_object_unref (task);
        return;
    }

    /* Allocate a new CID for the client to be created */
    if (cid == QMI_CID_NONE) {
        QmiMessageCtlAllocateCidInput *input;

        input = qmi_message_ctl_allocate_cid_input_new ();
        qmi_message_ctl_allocate_cid_input_set_service (input, ctx->service, NULL);

        g_debug ("[%s] Allocating new client ID...",
                 qmi_file_get_path_display (self->priv->file));
        qmi_client_ctl_allocate_cid (self->priv->client_ctl,
                                     input,
                                     timeout,
                                     cancellable,
                                     (GAsyncReadyCallback)allocate_cid_ready,
                                     task);

        qmi_message_ctl_allocate_cid_input_unref (input);
        return;
    }

    /* Reuse the given CID */
    g_debug ("[%s] Reusing client CID '%u'...",
             qmi_file_get_path_display (self->priv->file),
             cid);
    ctx->cid = cid;
    build_client_object (task);
}

/*****************************************************************************/
/* Release client */

gboolean
qmi_device_release_client_finish (QmiDevice *self,
                                  GAsyncResult *res,
                                  GError **error)
{
    return g_task_propagate_boolean (G_TASK (res), error);
}

static void
client_ctl_release_cid_ready (QmiClientCtl *client_ctl,
                              GAsyncResult *res,
                              GTask *task)
{
    GError *error = NULL;
    QmiMessageCtlReleaseCidOutput *output;

    /* Note: even if we return an error, the client is to be considered
     * released! (so shouldn't be used) */

    /* Check result of the async operation */
    output = qmi_client_ctl_release_cid_finish (client_ctl, res, &error);
    if (!output) {
        g_task_return_error (task, error);
        g_object_unref (task);
        return;
    }

    /* Check result of the QMI operation */
    if (!qmi_message_ctl_release_cid_output_get_result (output, &error)) {
        g_task_return_error (task, error);
        g_object_unref (task);
        qmi_message_ctl_release_cid_output_unref (output);
        return;
    }

    g_task_return_boolean (task, TRUE);
    g_object_unref (task);
    qmi_message_ctl_release_cid_output_unref (output);
}

void
qmi_device_release_client (QmiDevice *self,
                           QmiClient *client,
                           QmiDeviceReleaseClientFlags flags,
                           guint timeout,
                           GCancellable *cancellable,
                           GAsyncReadyCallback callback,
                           gpointer user_data)
{
    GTask *task;
    QmiService service;
    guint8 cid;
    gchar *flags_str;

    g_return_if_fail (QMI_IS_DEVICE (self));
    g_return_if_fail (QMI_IS_CLIENT (client));

    cid = qmi_client_get_cid (client);
    service = (guint8)qmi_client_get_service (client);

    /* The CTL client should not have been created out of the QmiDevice */
    g_return_if_fail (service != QMI_SERVICE_CTL);

    flags_str = qmi_device_release_client_flags_build_string_from_mask (flags);
    g_debug ("[%s] Releasing '%s' client with flags '%s'...",
             qmi_file_get_path_display (self->priv->file),
             qmi_service_get_string (service),
             flags_str);
    g_free (flags_str);

    task = g_task_new (self, cancellable, callback, user_data);

    /* Do not try to release an already released client */
    if (cid == QMI_CID_NONE) {
        g_task_return_new_error (task,
                                 QMI_CORE_ERROR,
                                 QMI_CORE_ERROR_INVALID_ARGS,
                                 "Client is already released");
        g_object_unref (task);
        return;
    }

    /* Keep the client object valid until after we reset its contents below */
    g_object_ref (client);

    /* Unregister from device */
    unregister_client (self, client);

    g_debug ("[%s] Unregistered '%s' client with ID '%u'",
             qmi_file_get_path_display (self->priv->file),
             qmi_service_get_string (service),
             cid);

    /* Reset the contents of the client object, making it invalid */
    g_object_set (client,
                  QMI_CLIENT_CID,     QMI_CID_NONE,
                  QMI_CLIENT_SERVICE, QMI_SERVICE_UNKNOWN,
                  QMI_CLIENT_DEVICE,  NULL,
                  NULL);

    g_object_unref (client);

    if (flags & QMI_DEVICE_RELEASE_CLIENT_FLAGS_RELEASE_CID) {
        QmiMessageCtlReleaseCidInput *input;

        /* And now, really try to release the CID */
        input = qmi_message_ctl_release_cid_input_new ();
        qmi_message_ctl_release_cid_input_set_release_info (input, service, cid, NULL);

        qmi_client_ctl_release_cid (self->priv->client_ctl,
                                    input,
                                    timeout,
                                    cancellable,
                                    (GAsyncReadyCallback)client_ctl_release_cid_ready,
                                    task);

        qmi_message_ctl_release_cid_input_unref (input);
        return;
    }

    /* No need to release the CID, so just done */
    g_task_return_boolean (task, TRUE);
    g_object_unref (task);
    return;
}

/*****************************************************************************/
/* Set instance ID */

gboolean
qmi_device_set_instance_id_finish (QmiDevice *self,
                                   GAsyncResult *res,
                                   guint16 *link_id,
                                   GError **error)
{
    gssize value;

    value = g_task_propagate_int (G_TASK (res), error);
    if (value == -1)
        return FALSE;

    if (link_id)
        *link_id = (guint16)value;

    return TRUE;
}

static void
set_instance_id_ready (QmiClientCtl *client_ctl,
                       GAsyncResult *res,
                       GTask *task)
{
    QmiMessageCtlSetInstanceIdOutput *output;
    GError *error = NULL;

    /* Check result of the async operation */
    output = qmi_client_ctl_set_instance_id_finish (client_ctl, res, &error);
    if (!output)
        g_task_return_error (task, error);
    else {
        /* Check result of the QMI operation */
        if (!qmi_message_ctl_set_instance_id_output_get_result (output, &error))
            g_task_return_error (task, error);
        else {
            guint16 link_id;

            qmi_message_ctl_set_instance_id_output_get_link_id (output, &link_id, NULL);
            g_task_return_int (task, link_id);
        }
        qmi_message_ctl_set_instance_id_output_unref (output);
    }

    g_object_unref (task);
}

void
qmi_device_set_instance_id (QmiDevice *self,
                            guint8 instance_id,
                            guint timeout,
                            GCancellable *cancellable,
                            GAsyncReadyCallback callback,
                            gpointer user_data)
{
    GTask *task;
    QmiMessageCtlSetInstanceIdInput *input;

    task = g_task_new (self, cancellable, callback, user_data);

    input = qmi_message_ctl_set_instance_id_input_new ();
    qmi_message_ctl_set_instance_id_input_set_id (
        input,
        instance_id,
        NULL);
    qmi_client_ctl_set_instance_id (self->priv->client_ctl,
                                    input,
                                    timeout,
                                    cancellable,
                                    (GAsyncReadyCallback)set_instance_id_ready,
                                    task);
    qmi_message_ctl_set_instance_id_input_unref (input);
}

/*****************************************************************************/
/* Input channel processing */

static void process_message (QmiMessage *message, QmiDevice *self);

static void
endpoint_new_data_cb (QmiEndpoint *endpoint,
                      QmiDevice   *self)
{
    GError *error = NULL;

    if (!qmi_endpoint_parse_buffer (endpoint,
                                    (QmiMessageHandler)process_message,
                                    self,
                                    &error)) {
        g_warning ("[%s] QMI parsing error: %s",
                   qmi_file_get_path_display (self->priv->file), error->message);
        g_error_free (error);
    }
}

static void
endpoint_hangup_cb (QmiEndpoint *endpoint,
                    QmiDevice   *self)
{
    g_signal_emit (self, signals[SIGNAL_REMOVED], 0);
}

typedef struct {
    QmiClient *client;
    QmiMessage *message;
} IdleIndicationContext;

static gboolean
process_indication_idle (IdleIndicationContext *ctx)
{
    g_assert (ctx->client != NULL);
    g_assert (ctx->message != NULL);

    __qmi_client_process_indication (ctx->client, ctx->message);

    g_object_unref (ctx->client);
    qmi_message_unref (ctx->message);
    g_slice_free (IdleIndicationContext, ctx);
    return FALSE;
}

static void
report_indication (QmiClient *client,
                   QmiMessage *message)
{
    IdleIndicationContext *ctx;
    GSource *source;

    /* Setup an idle to Pass the indication down to the client */
    ctx = g_slice_new (IdleIndicationContext);
    ctx->client = g_object_ref (client);
    ctx->message = qmi_message_ref (message);

    source = g_idle_source_new ();
    g_source_set_callback (source, (GSourceFunc)process_indication_idle, ctx, NULL);
    g_source_attach (source, g_main_context_get_thread_default ());
    g_source_unref (source);
}

static void
trace_message (QmiDevice         *self,
               QmiMessage        *message,
               gboolean           sent_or_received,
               const gchar       *message_str,
               QmiMessageContext *message_context)
{
    gchar       *printable;
    const gchar *prefix_str;
    const gchar *action_str;
    gchar       *vendor_str = NULL;

    if (!qmi_utils_get_traces_enabled ())
        return;

    if (sent_or_received) {
        prefix_str = "<<<<<< ";
        action_str = "sent";
    } else {
        prefix_str = "<<<<<< ";
        action_str = "received";
    }

    printable = __qmi_utils_str_hex (((GByteArray *)message)->data,
                                     ((GByteArray *)message)->len,
                                     ':');
    g_debug ("[%s] %s message...\n"
             "%sRAW:\n"
             "%s  length = %u\n"
             "%s  data   = %s\n",
             qmi_file_get_path_display (self->priv->file), action_str,
             prefix_str,
             prefix_str, ((GByteArray *)message)->len,
             prefix_str, printable);
    g_free (printable);

    if (message_context) {
        guint16 vendor_id;

        vendor_id = qmi_message_context_get_vendor_id (message_context);
        if (vendor_id != QMI_MESSAGE_VENDOR_GENERIC)
            vendor_str = g_strdup_printf ("vendor-specific (0x%04x)", vendor_id);
    }

    printable = qmi_message_get_printable_full (message, message_context, prefix_str);
    g_debug ("[%s] %s %s %s (translated)...\n%s",
             qmi_file_get_path_display (self->priv->file),
             action_str,
             vendor_str ? vendor_str : "generic",
             message_str,
             printable);
    g_free (printable);

    g_free (vendor_str);
}

static void
process_message (QmiMessage *message,
                 QmiDevice *self)
{
    if (qmi_message_is_indication (message)) {
        /* Indication traces translated without an explicit vendor */
        trace_message (self, message, FALSE, "indication", NULL);

        /* Generic emission of the indication */
        g_signal_emit (self, signals[SIGNAL_INDICATION], 0, message);

        if (qmi_message_get_client_id (message) == QMI_CID_BROADCAST) {
            GHashTableIter iter;
            gpointer key;
            QmiClient *client;

            g_hash_table_iter_init (&iter, self->priv->registered_clients);
            while (g_hash_table_iter_next (&iter, &key, (gpointer *)&client)) {
                /* For broadcast messages, report them just if the service matches */
                if (qmi_message_get_service (message) == qmi_client_get_service (client))
                    report_indication (client, message);
            }
        } else {
            QmiClient *client;

            client = g_hash_table_lookup (self->priv->registered_clients,
                                          build_registered_client_key (qmi_message_get_client_id (message),
                                                                       qmi_message_get_service (message)));
            if (client)
                report_indication (client, message);
        }

        return;
    }

    if (qmi_message_is_response (message)) {
        Transaction *tr;

        tr = device_match_transaction (self, message);
        if (!tr) {
            /* Unmatched transactions translated without an explicit context */
            trace_message (self, message, FALSE, "response", NULL);
            g_debug ("[%s] No transaction matched in received message",
                     qmi_file_get_path_display (self->priv->file));
        } else {
            /* Matched transactions translated with the same context as the request */
            trace_message (self, message, FALSE, "response", tr->message_context);
            /* Report the reply message */
            transaction_complete_and_free (tr, message, NULL);
        }

        return;
    }

    /* Unexpected message types translated without an explicit context */
    trace_message (self, message, FALSE, "unexpected message", NULL);
    g_debug ("[%s] Message received but it is neither an indication nor a response. Skipping it.",
             qmi_file_get_path_display (self->priv->file));
}

/*****************************************************************************/
/* Open device */

#define SYNC_TIMEOUT_SECS 2

typedef enum {
    DEVICE_OPEN_CONTEXT_STEP_FIRST = 0,
    DEVICE_OPEN_CONTEXT_STEP_DRIVER,
    DEVICE_OPEN_CONTEXT_STEP_CREATE_ENDPOINT,
    DEVICE_OPEN_CONTEXT_STEP_OPEN_ENDPOINT,
    DEVICE_OPEN_CONTEXT_STEP_FLAGS_VERSION_INFO,
    DEVICE_OPEN_CONTEXT_STEP_FLAGS_SYNC,
    DEVICE_OPEN_CONTEXT_STEP_FLAGS_NETPORT,
    DEVICE_OPEN_CONTEXT_STEP_FLAGS_EXPECT_INDICATIONS,
    DEVICE_OPEN_CONTEXT_STEP_LAST
} DeviceOpenContextStep;

typedef struct {
    DeviceOpenContextStep step;
    QmiDeviceOpenFlags flags;
    guint timeout;
    guint version_check_retries;
    guint sync_retries;
} DeviceOpenContext;

static void
device_open_context_free (DeviceOpenContext *ctx)
{
    g_slice_free (DeviceOpenContext, ctx);
}

gboolean
qmi_device_open_finish (QmiDevice *self,
                        GAsyncResult *res,
                        GError **error)
{
    return g_task_propagate_boolean (G_TASK (res), error);
}

static void device_open_step (GTask *task);

static void
setup_indications_ready (QmiEndpoint *endpoint,
                         GAsyncResult *res,
                         GTask *task)
{
    GError *error = NULL;
    DeviceOpenContext *ctx;

    ctx = g_task_get_task_data (task);

    if (!qmi_endpoint_setup_indications_finish (endpoint, res, &error)) {
        g_task_return_error (task, error);
        g_object_unref (task);
        return;
    }

    ctx->step++;
    device_open_step (task);
}

static void
ctl_set_data_format_ready (QmiClientCtl *client,
                           GAsyncResult *res,
                           GTask *task)
{
    QmiDevice *self;
    DeviceOpenContext *ctx;
    QmiMessageCtlSetDataFormatOutput *output = NULL;
    GError *error = NULL;

    output = qmi_client_ctl_set_data_format_finish (client, res, &error);
    /* Check result of the async operation */
    if (!output) {
        g_task_return_error (task, error);
        g_object_unref (task);
        return;
    }

    /* Check result of the QMI operation */
    if (!qmi_message_ctl_set_data_format_output_get_result (output, &error)) {
        g_task_return_error (task, error);
        g_object_unref (task);
        qmi_message_ctl_set_data_format_output_unref (output);
        return;
    }

    self = g_task_get_source_object (task);

    g_debug ("[%s] Network port data format operation finished",
             qmi_file_get_path_display (self->priv->file));

    qmi_message_ctl_set_data_format_output_unref (output);

    /* Go on */
    ctx = g_task_get_task_data (task);
    ctx->step++;
    device_open_step (task);
}

static void
sync_ready (QmiClientCtl *client_ctl,
            GAsyncResult *res,
            GTask *task)
{
    QmiDevice *self;
    DeviceOpenContext *ctx;
    GError *error = NULL;
    QmiMessageCtlSyncOutput *output;

    self = g_task_get_source_object (task);
    ctx  = g_task_get_task_data (task);

    /* Check result of the async operation */
    output = qmi_client_ctl_sync_finish (client_ctl, res, &error);
    if (!output) {
        if (g_error_matches (error, QMI_CORE_ERROR, QMI_CORE_ERROR_TIMEOUT)) {
            /* Update retries... */
            ctx->sync_retries--;
            /* If retries left, retry */
            if (ctx->sync_retries > 0) {
                g_error_free (error);
                qmi_client_ctl_sync (self->priv->client_ctl,
                                     NULL,
                                     SYNC_TIMEOUT_SECS,
                                     g_task_get_cancellable (task),
                                     (GAsyncReadyCallback)sync_ready,
                                     task);
                return;
            }

            /* Otherwise, propagate the error */
        }
        g_task_return_error (task, error);
        g_object_unref (task);
        return;
    }

    /* Check result of the QMI operation */
    if (!qmi_message_ctl_sync_output_get_result (output, &error)) {
        g_task_return_error (task, error);
        g_object_unref (task);
        qmi_message_ctl_sync_output_unref (output);
        return;
    }

    g_debug ("[%s] Sync operation finished",
             qmi_file_get_path_display (self->priv->file));

    qmi_message_ctl_sync_output_unref (output);

    /* Go on */
    ctx->step++;
    device_open_step (task);
}

static void
open_version_info_ready (QmiClientCtl *client_ctl,
                         GAsyncResult *res,
                         GTask *task)
{
    QmiDevice *self;
    DeviceOpenContext *ctx;
    GArray *service_list;
    QmiMessageCtlGetVersionInfoOutput *output;
    GError *error = NULL;
    guint i;

    self = g_task_get_source_object (task);
    ctx = g_task_get_task_data (task);

    /* Check result of the async operation */
    output = qmi_client_ctl_get_version_info_finish (client_ctl, res, &error);
    if (!output) {
        if (g_error_matches (error, QMI_CORE_ERROR, QMI_CORE_ERROR_TIMEOUT)) {
            /* Update retries... */
            ctx->version_check_retries--;
            /* If retries left, retry */
            if (ctx->version_check_retries > 0) {
                g_error_free (error);
                qmi_client_ctl_get_version_info (self->priv->client_ctl,
                                                 NULL,
                                                 1,
                                                 g_task_get_cancellable (task),
                                                 (GAsyncReadyCallback)open_version_info_ready,
                                                 task);
                return;
            }

            /* Otherwise, propagate the error */
        }

        g_task_return_error (task, error);
        g_object_unref (task);
        return;
    }

    /* Check result of the QMI operation */
    if (!qmi_message_ctl_get_version_info_output_get_result (output, &error)) {
        g_task_return_error (task, error);
        g_object_unref (task);
        qmi_message_ctl_get_version_info_output_unref (output);
        return;
    }

    /* QMI operation succeeded, we can now get the outputs */
    service_list = NULL;
    qmi_message_ctl_get_version_info_output_get_service_list (output,
                                                              &service_list,
                                                              NULL);
    self->priv->supported_services = g_array_ref (service_list);

    g_debug ("[%s] QMI Device supports %u services:",
             qmi_file_get_path_display (self->priv->file),
             self->priv->supported_services->len);
    for (i = 0; i < self->priv->supported_services->len; i++) {
        QmiMessageCtlGetVersionInfoOutputServiceListService *info;
        const gchar *service_str;

        info = &g_array_index (self->priv->supported_services,
                               QmiMessageCtlGetVersionInfoOutputServiceListService,
                               i);
        service_str = qmi_service_get_string (info->service);
        if (service_str)
            g_debug ("[%s]    %s (%u.%u)",
                     qmi_file_get_path_display (self->priv->file),
                     service_str,
                     info->major_version,
                     info->minor_version);
        else
            g_debug ("[%s]    unknown [0x%02x] (%u.%u)",
                     qmi_file_get_path_display (self->priv->file),
                     info->service,
                     info->major_version,
                     info->minor_version);
    }

    qmi_message_ctl_get_version_info_output_unref (output);

    /* Go on */
    ctx->step++;
    device_open_step (task);
}

static void
endpoint_ready (QmiEndpoint *endpoint,
                GAsyncResult *res,
                GTask *task)
{
    GError *error = NULL;
    DeviceOpenContext *ctx;

    ctx = g_task_get_task_data (task);

    if (!qmi_endpoint_open_finish (endpoint, res, &error)) {
        g_task_return_error (task, error);
        g_object_unref (task);
        return;
    }

    ctx->step++;
    device_open_step (task);
}

#define NETPORT_FLAGS (QMI_DEVICE_OPEN_FLAGS_NET_802_3 | \
                       QMI_DEVICE_OPEN_FLAGS_NET_RAW_IP | \
                       QMI_DEVICE_OPEN_FLAGS_NET_QOS_HEADER | \
                       QMI_DEVICE_OPEN_FLAGS_NET_NO_QOS_HEADER)

static void
device_create_endpoint (QmiDevice *self,
                        DeviceOpenContext *ctx)
{
    if (!(ctx->flags & QMI_DEVICE_OPEN_FLAGS_MBIM))
        self->priv->endpoint = QMI_ENDPOINT (qmi_endpoint_qmux_new (self->priv->file,
                                                                    self->priv->proxy_path,
                                                                    self->priv->client_ctl));
#if defined MBIM_QMUX_ENABLED
    else
        self->priv->endpoint = QMI_ENDPOINT (qmi_endpoint_mbim_new (self->priv->file));
#endif /* MBIM_QMUX_ENABLED */

    if (!self->priv->endpoint)
        return;

    self->priv->endpoint_new_data_id = g_signal_connect (self->priv->endpoint,
                                                         QMI_ENDPOINT_SIGNAL_NEW_DATA,
                                                         G_CALLBACK (endpoint_new_data_cb),
                                                         self);
    self->priv->endpoint_hangup_id = g_signal_connect (self->priv->endpoint,
                                                       QMI_ENDPOINT_SIGNAL_HANGUP,
                                                       G_CALLBACK (endpoint_hangup_cb),
                                                       self);
    g_debug ("[%s] created endpoint", qmi_file_get_path_display (self->priv->file));
}

static gboolean
device_setup_open_flags_by_driver (QmiDevice          *self,
                                   DeviceOpenContext  *ctx,
                                   GError            **error)
{
    gchar  *driver;
    GError *inner_error = NULL;

    driver = __qmi_utils_get_driver (qmi_file_get_path (self->priv->file), &inner_error);
    if (driver)
        g_debug ("[%s] loaded driver of cdc-wdm port: %s", qmi_file_get_path_display (self->priv->file), driver);
    else if (!self->priv->no_file_check)
        g_warning ("[%s] couldn't load driver of cdc-wdm port: %s", qmi_file_get_path_display (self->priv->file), inner_error->message);
    g_clear_error (&inner_error);

#if defined MBIM_QMUX_ENABLED

    /* Auto mode requested? */
    if (ctx->flags & QMI_DEVICE_OPEN_FLAGS_AUTO) {
        if (!g_strcmp0 (driver, "cdc_mbim")) {
            g_debug ("[%s] automatically selecting MBIM mode", qmi_file_get_path_display (self->priv->file));
            ctx->flags |= QMI_DEVICE_OPEN_FLAGS_MBIM;
            goto out;
        }
        if (!g_strcmp0 (driver, "qmi_wwan")) {
            g_debug ("[%s] automatically selecting QMI mode", qmi_file_get_path_display (self->priv->file));
            ctx->flags &= ~QMI_DEVICE_OPEN_FLAGS_MBIM;
            goto out;
        }
        g_set_error (&inner_error,
                     QMI_CORE_ERROR,
                     QMI_CORE_ERROR_FAILED,
                     "Cannot automatically select QMI/MBIM mode: driver %s",
                     driver ? driver : "unknown");
        goto out;
    }

    /* MBIM mode requested? */
    if (ctx->flags & QMI_DEVICE_OPEN_FLAGS_MBIM) {
        if (g_strcmp0 (driver, "cdc_mbim") && !self->priv->no_file_check)
            g_warning ("[%s] requested MBIM mode but unexpected driver found: %s", qmi_file_get_path_display (self->priv->file), driver);
        goto out;
    }

#else

    if (ctx->flags & QMI_DEVICE_OPEN_FLAGS_AUTO)
        g_warning ("[%s] requested auto mode but no MBIM QMUX support available", qmi_file_get_path_display (self->priv->file));
    if (ctx->flags & QMI_DEVICE_OPEN_FLAGS_MBIM)
        g_warning ("[%s] requested MBIM mode but no MBIM QMUX support available", qmi_file_get_path_display (self->priv->file));

#endif /* MBIM_QMUX_ENABLED */

    /* QMI mode requested? */
    if (g_strcmp0 (driver, "qmi_wwan") && !self->priv->no_file_check)
        g_warning ("[%s] requested QMI mode but unexpected driver found: %s",
                   qmi_file_get_path_display (self->priv->file), driver ? driver : "unknown");

#if defined MBIM_QMUX_ENABLED
out:
#endif

    g_free (driver);

    if (inner_error) {
        g_propagate_error (error, inner_error);
        return FALSE;
    }

    return TRUE;
}

static void
device_open_step (GTask *task)
{
    QmiDevice *self;
    DeviceOpenContext *ctx;
    GError *error = NULL;

    self = g_task_get_source_object (task);
    ctx = g_task_get_task_data (task);

    switch (ctx->step) {
    case DEVICE_OPEN_CONTEXT_STEP_FIRST:
        ctx->step++;
        /* Fall through */

    case DEVICE_OPEN_CONTEXT_STEP_DRIVER:
        if (!device_setup_open_flags_by_driver (self, ctx, &error)) {
            g_task_return_error (task, error);
            g_object_unref (task);
            return;
        }
        ctx->step++;
        /* Fall through */

    case DEVICE_OPEN_CONTEXT_STEP_CREATE_ENDPOINT:
        device_create_endpoint (self, ctx);
        if (!self->priv->endpoint) {
            g_task_return_new_error (task,
                                     QMI_CORE_ERROR,
                                     QMI_CORE_ERROR_FAILED,
                                     "Could not create endpoint");
            g_object_unref (task);
            return;
        }
        ctx->step++;
        /* Fall through */

    case DEVICE_OPEN_CONTEXT_STEP_OPEN_ENDPOINT:
        qmi_endpoint_open (self->priv->endpoint,
                           !!(ctx->flags & QMI_DEVICE_OPEN_FLAGS_PROXY),
                           5,
                           g_task_get_cancellable (task),
                           (GAsyncReadyCallback)endpoint_ready,
                           task);
        return;

    case DEVICE_OPEN_CONTEXT_STEP_FLAGS_VERSION_INFO:
        /* Query version info? */
        if (ctx->flags & QMI_DEVICE_OPEN_FLAGS_VERSION_INFO) {
            /* Setup how many times to retry... We'll retry once per second */
            ctx->version_check_retries = ctx->timeout > 0 ? ctx->timeout : 1;
            g_debug ("[%s] Checking version info (%u retries)...",
                     qmi_file_get_path_display (self->priv->file),
                     ctx->version_check_retries);
            qmi_client_ctl_get_version_info (self->priv->client_ctl,
                                             NULL,
                                             1,
                                             g_task_get_cancellable (task),
                                             (GAsyncReadyCallback)open_version_info_ready,
                                             task);
            return;
        }
        ctx->step++;
        /* Fall through */

    case DEVICE_OPEN_CONTEXT_STEP_FLAGS_SYNC:
        /* Sync? */
        if (ctx->flags & QMI_DEVICE_OPEN_FLAGS_SYNC) {
            /* Setup how many times to retry... We'll retry once per second */
            ctx->sync_retries = ctx->timeout > SYNC_TIMEOUT_SECS ? (ctx->timeout / SYNC_TIMEOUT_SECS) : 1;
            g_debug ("[%s] Running sync (%u retries)...",
                     qmi_file_get_path_display (self->priv->file),
                     ctx->sync_retries);
            qmi_client_ctl_sync (self->priv->client_ctl,
                                 NULL,
                                 SYNC_TIMEOUT_SECS,
                                 g_task_get_cancellable (task),
                                 (GAsyncReadyCallback)sync_ready,
                                 task);
            return;
        }
        ctx->step++;
        /* Fall through */

    case DEVICE_OPEN_CONTEXT_STEP_FLAGS_NETPORT:
        /* Network port setup */
        if (ctx->flags & NETPORT_FLAGS) {
            QmiMessageCtlSetDataFormatInput *input;
            QmiCtlDataFormat qos = QMI_CTL_DATA_FORMAT_QOS_FLOW_HEADER_ABSENT;
            QmiCtlDataLinkProtocol link_protocol = QMI_CTL_DATA_LINK_PROTOCOL_802_3;

            g_debug ("[%s] Setting network port data format...",
                     qmi_file_get_path_display (self->priv->file));

            input = qmi_message_ctl_set_data_format_input_new ();

            if (ctx->flags & QMI_DEVICE_OPEN_FLAGS_NET_QOS_HEADER)
                qos = QMI_CTL_DATA_FORMAT_QOS_FLOW_HEADER_PRESENT;
            qmi_message_ctl_set_data_format_input_set_format (input, qos, NULL);

            if (ctx->flags & QMI_DEVICE_OPEN_FLAGS_NET_RAW_IP)
                link_protocol = QMI_CTL_DATA_LINK_PROTOCOL_RAW_IP;
            qmi_message_ctl_set_data_format_input_set_protocol (input, link_protocol, NULL);

            qmi_client_ctl_set_data_format (self->priv->client_ctl,
                                            input,
                                            5,
                                            NULL,
                                            (GAsyncReadyCallback)ctl_set_data_format_ready,
                                            task);
            qmi_message_ctl_set_data_format_input_unref (input);
            return;
        }
        ctx->step++;
        /* Fall through */

    case DEVICE_OPEN_CONTEXT_STEP_FLAGS_EXPECT_INDICATIONS:
        qmi_endpoint_setup_indications (self->priv->endpoint,
                                        10,
                                        g_task_get_cancellable (task),
                                        (GAsyncReadyCallback)setup_indications_ready,
                                        task);
        return;
        /* Fall through */

    case DEVICE_OPEN_CONTEXT_STEP_LAST:
        /* Nothing else to process, done we are */
        g_task_return_boolean (task, TRUE);
        g_object_unref (task);
        return;

    default:
        break;
    }

    g_assert_not_reached ();
}

void
qmi_device_open (QmiDevice *self,
                 QmiDeviceOpenFlags flags,
                 guint timeout,
                 GCancellable *cancellable,
                 GAsyncReadyCallback callback,
                 gpointer user_data)
{
    DeviceOpenContext *ctx;
    gchar *flags_str;
    GTask *task;

    /* Raw IP and 802.3 are mutually exclusive */
    g_return_if_fail (!((flags & QMI_DEVICE_OPEN_FLAGS_NET_802_3) &&
                        (flags & QMI_DEVICE_OPEN_FLAGS_NET_RAW_IP)));
    /* QoS and no QoS are mutually exclusive */
    g_return_if_fail (!((flags & QMI_DEVICE_OPEN_FLAGS_NET_QOS_HEADER) &&
                        (flags & QMI_DEVICE_OPEN_FLAGS_NET_NO_QOS_HEADER)));
    /* At least one of both link protocol and QoS must be specified */
    if (flags & (QMI_DEVICE_OPEN_FLAGS_NET_802_3 | QMI_DEVICE_OPEN_FLAGS_NET_RAW_IP))
        g_return_if_fail (flags & (QMI_DEVICE_OPEN_FLAGS_NET_QOS_HEADER | QMI_DEVICE_OPEN_FLAGS_NET_NO_QOS_HEADER));
    if (flags & (QMI_DEVICE_OPEN_FLAGS_NET_QOS_HEADER | QMI_DEVICE_OPEN_FLAGS_NET_NO_QOS_HEADER))
        g_return_if_fail (flags & (QMI_DEVICE_OPEN_FLAGS_NET_802_3 | QMI_DEVICE_OPEN_FLAGS_NET_RAW_IP));

    g_return_if_fail (QMI_IS_DEVICE (self));

    flags_str = qmi_device_open_flags_build_string_from_mask (flags);
    g_debug ("[%s] Opening device with flags '%s'...",
             qmi_file_get_path_display (self->priv->file),
             flags_str);
    g_free (flags_str);

    ctx = g_slice_new (DeviceOpenContext);
    ctx->step = DEVICE_OPEN_CONTEXT_STEP_FIRST;
    ctx->flags = flags;
    ctx->timeout = timeout;

    task = g_task_new (self, cancellable, callback, user_data);
    g_task_set_task_data (task, ctx, (GDestroyNotify)device_open_context_free);

    /* Start processing */
    device_open_step (task);
}

/*****************************************************************************/
/* Close stream */

gboolean
qmi_device_close_finish (QmiDevice     *self,
                         GAsyncResult  *res,
                         GError       **error)
{
    return g_task_propagate_boolean (G_TASK (res), error);
}

static void
endpoint_cleanup (QmiDevice *self)
{
    if (!self->priv->endpoint)
        return;

    if (self->priv->endpoint_hangup_id) {
        g_signal_handler_disconnect (self->priv->endpoint, self->priv->endpoint_hangup_id);
        self->priv->endpoint_hangup_id = 0;
    }
    if (self->priv->endpoint_new_data_id) {
        g_signal_handler_disconnect (self->priv->endpoint, self->priv->endpoint_new_data_id);
        self->priv->endpoint_new_data_id = 0;
    }
    g_clear_object (&self->priv->endpoint);
}

static void
endpoint_close_ready (QmiEndpoint  *endpoint,
                      GAsyncResult *res,
                      GTask        *task)
{
    QmiDevice *self;
    GError    *error = NULL;

    self = g_task_get_source_object (task);

    qmi_endpoint_close_finish (endpoint, res, &error);

    /* Success or failure, we want to get rid of this endpoint. */
    endpoint_cleanup (self);

    if (error)
        g_task_return_error (task, error);
    else
        g_task_return_boolean (task, TRUE);
    g_object_unref (task);
}

void
qmi_device_close_async (QmiDevice           *self,
                        guint                timeout,
                        GCancellable        *cancellable,
                        GAsyncReadyCallback  callback,
                        gpointer             user_data)
{
    GTask *task;

    task = g_task_new (self, cancellable, callback, user_data);

    /* if already closed, we're done */
    if (!self->priv->endpoint) {
        g_task_return_boolean (task, TRUE);
        g_object_unref (task);
        return;
    }

    qmi_endpoint_close (self->priv->endpoint,
                        timeout,
                        cancellable,
                        (GAsyncReadyCallback)endpoint_close_ready,
                        task);
}

/*****************************************************************************/
/* Command */

QmiMessage *
qmi_device_command_abortable_finish (QmiDevice     *self,
                                     GAsyncResult  *res,
                                     GError       **error)
{
    if (g_simple_async_result_propagate_error (G_SIMPLE_ASYNC_RESULT (res), error))
        return NULL;

    return qmi_message_ref (g_simple_async_result_get_op_res_gpointer (
                                G_SIMPLE_ASYNC_RESULT (res)));
}

static void
transaction_early_error (QmiDevice   *self,
                         Transaction *tr,
                         gboolean     stored,
                         GError      *error)
{
    g_assert (error);

    if (stored) {
        /* Match transaction so that we remove it from our tracking table */
        tr = device_match_transaction (self, tr->message);
        g_assert (tr);
    }
    transaction_complete_and_free (tr, NULL, error);
    g_error_free (error);
}

void
qmi_device_command_abortable (QmiDevice                                *self,
                              QmiMessage                               *message,
                              QmiMessageContext                        *message_context,
                              guint                                     timeout,
                              QmiDeviceCommandAbortableBuildRequestFn   abort_build_request_fn,
                              QmiDeviceCommandAbortableParseResponseFn  abort_parse_response_fn,
                              gpointer                                  abort_user_data,
                              GDestroyNotify                            abort_user_data_free,
                              GCancellable                             *cancellable,
                              GAsyncReadyCallback                       callback,
                              gpointer                                  user_data)
{
    GError *error = NULL;
    Transaction *tr;

    g_return_if_fail (QMI_IS_DEVICE (self));
    g_return_if_fail (message != NULL);
    g_return_if_fail (timeout > 0);

    /* either none or both set */
    g_return_if_fail ((!abort_build_request_fn && !abort_parse_response_fn) ||
                      (abort_build_request_fn  && abort_parse_response_fn));

    /* Use a proper transaction id for CTL messages if they don't have one */
    if (qmi_message_get_service (message) == QMI_SERVICE_CTL &&
        qmi_message_get_transaction_id (message) == 0) {
        qmi_message_set_transaction_id (
            message,
            qmi_client_get_next_transaction_id (
                QMI_CLIENT (
                    self->priv->client_ctl)));
    }

    tr = transaction_new (self, message, message_context, cancellable, callback, user_data);

    /* Device must be open */
    if (!qmi_endpoint_is_open (self->priv->endpoint)) {
        error = g_error_new (QMI_CORE_ERROR,
                             QMI_CORE_ERROR_WRONG_STATE,
                             "Device must be open to send commands");
        transaction_early_error (self, tr, FALSE, error);
        return;
    }

    /* Non-CTL services should use a proper CID */
    if (qmi_message_get_service (message) != QMI_SERVICE_CTL &&
        qmi_message_get_client_id (message) == 0) {
        error = g_error_new (QMI_CORE_ERROR,
                             QMI_CORE_ERROR_FAILED,
                             "Cannot send message in service '%s' without a CID",
                             qmi_service_get_string (qmi_message_get_service (message)));
        transaction_early_error (self, tr, FALSE, error);
        return;
    }

    /* Check if the message to be sent is supported by the device
     * (only applicable if we did version info check when opening) */
    if (!check_message_supported (self, message, &error)) {
        g_prefix_error (&error, "Cannot send message: ");
        transaction_early_error (self, tr, FALSE, error);
        return;
    }

    /* If message is not abortable, we should not allow using the abortable() interface */
    if (!__qmi_message_is_abortable (message, message_context)) {
        if (abort_build_request_fn || abort_parse_response_fn) {
            error = g_error_new (QMI_CORE_ERROR,
                                 QMI_CORE_ERROR_FAILED,
                                 "Message is not abortable");
            transaction_early_error (self, tr, FALSE, error);
            return;
        }
    } else {
        /* Store abortable info, if any */
        tr->abort_build_request_fn  = abort_build_request_fn;
        tr->abort_parse_response_fn = abort_parse_response_fn;
        tr->abort_user_data         = abort_user_data;
        tr->abort_user_data_free    = abort_user_data_free;
    }

    /* Setup context to match response */
    if (!device_store_transaction (self, tr, timeout, &error)) {
        g_prefix_error (&error, "Cannot store transaction: ");
        transaction_early_error (self, tr, FALSE, error);
        return;
    }

    /* From now on, if we want to complete the transaction with an early error,
     *  it needs to be removed from the tracking table as well. */

    trace_message (self, message, TRUE, "request", message_context);

    if (!qmi_endpoint_send (self->priv->endpoint, message, timeout, cancellable, &error)) {
        transaction_early_error (self, tr, TRUE, error);
        return;
    }
}

/*****************************************************************************/
/* Non-abortable standard command */

QmiMessage *
qmi_device_command_full_finish (QmiDevice     *self,
                                GAsyncResult  *res,
                                GError       **error)
{
    return qmi_device_command_abortable_finish (self, res, error);
}

void
qmi_device_command_full (QmiDevice           *self,
                         QmiMessage          *message,
                         QmiMessageContext   *message_context,
                         guint                timeout,
                         GCancellable        *cancellable,
                         GAsyncReadyCallback  callback,
                         gpointer             user_data)
{
    qmi_device_command_abortable (self,
                                  message,
                                  message_context,
                                  timeout,
                                  NULL, /* abort_build_request_fn  */
                                  NULL, /* abort_parse_response_fn */
                                  NULL, /* abort_user_data         */
                                  NULL, /* abort_user_data_free    */
                                  cancellable,
                                  callback,
                                  user_data);
}

/*****************************************************************************/
/* Generic command */

QmiMessage *
qmi_device_command_finish (QmiDevice     *self,
                           GAsyncResult  *res,
                           GError       **error)
{
    return qmi_device_command_full_finish (self, res, error);
}

void
qmi_device_command (QmiDevice           *self,
                    QmiMessage          *message,
                    guint                timeout,
                    GCancellable        *cancellable,
                    GAsyncReadyCallback  callback,
                    gpointer             user_data)
{
    qmi_device_command_full (self, message, NULL, timeout, cancellable, callback, user_data);
}

/*****************************************************************************/
/* New QMI device */

QmiDevice *
qmi_device_new_finish (GAsyncResult *res,
                       GError **error)
{
    GObject *ret;
    GObject *source_object;

    source_object = g_async_result_get_source_object (res);
    ret = g_async_initable_new_finish (G_ASYNC_INITABLE (source_object), res, error);
    g_object_unref (source_object);

    return (ret ? QMI_DEVICE (ret) : NULL);
}

void
qmi_device_new (GFile *file,
                GCancellable *cancellable,
                GAsyncReadyCallback callback,
                gpointer user_data)
{
    g_async_initable_new_async (QMI_TYPE_DEVICE,
                                G_PRIORITY_DEFAULT,
                                cancellable,
                                callback,
                                user_data,
                                QMI_DEVICE_FILE, file,
                                NULL);
}

/*****************************************************************************/
/* Async init */

static gboolean
initable_init_finish (GAsyncInitable  *initable,
                      GAsyncResult    *result,
                      GError         **error)
{
    return g_task_propagate_boolean (G_TASK (result), error);
}

static void
sync_indication_cb (QmiClientCtl *client_ctl,
                    QmiDevice *self)
{
    /* Just log about it */
    g_debug ("[%s] Sync indication received",
             qmi_file_get_path_display (self->priv->file));
}

static void
client_ctl_setup (GTask *task)
{
    QmiDevice *self;
    GError *error = NULL;

    self = g_task_get_source_object (task);

    /* Create the implicit CTL client */
    self->priv->client_ctl = g_object_new (QMI_TYPE_CLIENT_CTL,
                                           QMI_CLIENT_DEVICE,  self,
                                           QMI_CLIENT_SERVICE, QMI_SERVICE_CTL,
                                           QMI_CLIENT_CID,     QMI_CID_NONE,
                                           NULL);

    /* Register the CTL client to get indications */
    register_client (self,
                     QMI_CLIENT (self->priv->client_ctl),
                     &error);
    g_assert_no_error (error);

    /* Connect to 'Sync' indications */
    self->priv->sync_indication_id =
        g_signal_connect (self->priv->client_ctl,
                          "sync",
                          G_CALLBACK (sync_indication_cb),
                          self);

    /* Done we are */
    g_task_return_boolean (task, TRUE);
    g_object_unref (task);
}

static void
check_type_async_ready (QmiFile *file,
                        GAsyncResult *res,
                        GTask *task)
{
    GError *error = NULL;

    if (!qmi_file_check_type_finish (file, res, &error)) {
        g_task_return_error (task, error);
        g_object_unref (task);
        return;
    }

    /* Go on with client CTL setup */
    client_ctl_setup (task);
}

static void
initable_init_async (GAsyncInitable *initable,
                     int io_priority,
                     GCancellable *cancellable,
                     GAsyncReadyCallback callback,
                     gpointer user_data)
{
    QmiDevice *self;
    GTask *task;

    self = QMI_DEVICE (initable);
    task = g_task_new (self, cancellable, callback, user_data);

    /* We need a proper file to initialize */
    if (!self->priv->file) {
        g_task_return_new_error (task,
                                 QMI_CORE_ERROR,
                                 QMI_CORE_ERROR_INVALID_ARGS,
                                 "Cannot initialize QMI device: No file given");
        g_object_unref (task);
        return;
    }

    /* If no file check requested, don't do it */
    if (self->priv->no_file_check) {
        client_ctl_setup (task);
        return;
    }

    /* Check the file type. Note that this is just a quick check to avoid
     * creating QmiDevices pointing to a location already known not to be a QMI
     * device. */
    qmi_file_check_type_async (self->priv->file,
                               cancellable,
                               (GAsyncReadyCallback)check_type_async_ready,
                               task);
}

/*****************************************************************************/

static void
set_property (GObject *object,
              guint prop_id,
              const GValue *value,
              GParamSpec *pspec)
{
    QmiDevice *self = QMI_DEVICE (object);

    switch (prop_id) {
    case PROP_FILE:
        g_assert (self->priv->file == NULL);
        self->priv->file = qmi_file_new (g_value_get_object (value));
        break;
    case PROP_NO_FILE_CHECK:
        self->priv->no_file_check = g_value_get_boolean (value);
        break;
    case PROP_PROXY_PATH:
        g_free (self->priv->proxy_path);
        self->priv->proxy_path = g_value_dup_string (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
get_property (GObject *object,
              guint prop_id,
              GValue *value,
              GParamSpec *pspec)
{
    QmiDevice *self = QMI_DEVICE (object);

    switch (prop_id) {
    case PROP_FILE:
        g_value_set_object (value, qmi_file_get_file (self->priv->file));
        break;
    case PROP_WWAN_IFACE:
        reload_wwan_iface_name (self);
        g_value_set_string (value, self->priv->wwan_iface);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
qmi_device_init (QmiDevice *self)
{
    self->priv = G_TYPE_INSTANCE_GET_PRIVATE ((self),
                                              QMI_TYPE_DEVICE,
                                              QmiDevicePrivate);

    self->priv->transactions = g_hash_table_new (g_direct_hash,
                                                 g_direct_equal);

    self->priv->registered_clients = g_hash_table_new_full (g_direct_hash,
                                                            g_direct_equal,
                                                            NULL,
                                                            g_object_unref);
    self->priv->proxy_path = g_strdup (QMI_PROXY_SOCKET_PATH);
}

static gboolean
foreach_warning (gpointer key,
                 QmiClient *client,
                 QmiDevice *self)
{
    g_warning ("[%s] QMI client for service '%s' with CID '%u' wasn't released",
               qmi_file_get_path_display (self->priv->file),
               qmi_service_get_string (qmi_client_get_service (client)),
               qmi_client_get_cid (client));

    return TRUE;
}

static void
dispose (GObject *object)
{
    QmiDevice *self = QMI_DEVICE (object);

    /* unregister our CTL client */
    if (self->priv->client_ctl)
        unregister_client (self, QMI_CLIENT (self->priv->client_ctl));

    /* If clients were left unreleased, we'll just warn about it.
     * There is no point in trying to request CID releases, as the device
     * itself is being disposed. */
    g_hash_table_foreach_remove (self->priv->registered_clients,
                                 (GHRFunc)foreach_warning,
                                 self);

    if (self->priv->sync_indication_id &&
        self->priv->client_ctl) {
        g_signal_handler_disconnect (self->priv->client_ctl,
                                     self->priv->sync_indication_id);
        self->priv->sync_indication_id = 0;
    }
    g_clear_object (&self->priv->client_ctl);

    endpoint_cleanup (self);
    g_clear_object (&self->priv->file);

    G_OBJECT_CLASS (qmi_device_parent_class)->dispose (object);
}

static void
finalize (GObject *object)
{
    QmiDevice *self = QMI_DEVICE (object);

    /* Transactions keep refs to the device, so it's actually
     * impossible to have any content in the HT */
    if (self->priv->transactions) {
        g_assert (g_hash_table_size (self->priv->transactions) == 0);
        g_hash_table_unref (self->priv->transactions);
    }

    g_hash_table_unref (self->priv->registered_clients);

    if (self->priv->supported_services)
        g_array_unref (self->priv->supported_services);

    g_free (self->priv->proxy_path);
    g_free (self->priv->wwan_iface);

    G_OBJECT_CLASS (qmi_device_parent_class)->finalize (object);
}

static void
async_initable_iface_init (GAsyncInitableIface *iface)
{
    iface->init_async = initable_init_async;
    iface->init_finish = initable_init_finish;
}

static void
qmi_device_class_init (QmiDeviceClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (object_class, sizeof (QmiDevicePrivate));

    object_class->get_property = get_property;
    object_class->set_property = set_property;
    object_class->finalize = finalize;
    object_class->dispose = dispose;

    /**
     * QmiDevice:device-file:
     *
     * Since: 1.0
     */
    properties[PROP_FILE] =
        g_param_spec_object (QMI_DEVICE_FILE,
                             "Device file",
                             "File to the underlying QMI device",
                             G_TYPE_FILE,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
    g_object_class_install_property (object_class, PROP_FILE, properties[PROP_FILE]);

    /**
     * QmiDevice:device-no-file-check:
     *
     * Since: 1.12
     */
    properties[PROP_NO_FILE_CHECK] =
        g_param_spec_boolean (QMI_DEVICE_NO_FILE_CHECK,
                              "No file check",
                              "Don't check for file existence when creating the Qmi device.",
                              FALSE,
                              G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
    g_object_class_install_property (object_class, PROP_NO_FILE_CHECK, properties[PROP_NO_FILE_CHECK]);

    /**
     * QmiDevice:device-proxy-path:
     *
     * Since: 1.12
     */
    properties[PROP_PROXY_PATH] =
        g_param_spec_string (QMI_DEVICE_PROXY_PATH,
                             "Proxy path",
                             "Path of the abstract socket where the proxy is available.",
                             QMI_PROXY_SOCKET_PATH,
                             G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
    g_object_class_install_property (object_class, PROP_PROXY_PATH, properties[PROP_PROXY_PATH]);

    /**
     * QmiDevice:device-wwan-iface:
     *
     * Since: 1.14
     */
    properties[PROP_WWAN_IFACE] =
        g_param_spec_string (QMI_DEVICE_WWAN_IFACE,
                             "WWAN iface",
                             "Name of the WWAN network interface associated with the control port.",
                             NULL,
                             G_PARAM_READABLE);
    g_object_class_install_property (object_class, PROP_WWAN_IFACE, properties[PROP_WWAN_IFACE]);

    /**
     * QmiDevice::indication:
     * @object: A #QmiDevice.
     * @output: A #QmiMessage.
     *
     * The ::indication signal gets emitted when a QMI indication is received.
     *
     * Since: 1.8
     */
    signals[SIGNAL_INDICATION] =
        g_signal_new (QMI_DEVICE_SIGNAL_INDICATION,
                      G_OBJECT_CLASS_TYPE (G_OBJECT_CLASS (klass)),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL,
                      NULL,
                      NULL,
                      G_TYPE_NONE,
                      1,
                      G_TYPE_BYTE_ARRAY);

    /**
     * QmiDevice::device-removed:
     * @object: A #QmiDevice.
     * @output: none
     *
     * The ::device-removed signal is emitted when an unexpected port hang-up is received.
     *
     * Since: 1.20
     */
    signals[SIGNAL_REMOVED] =
        g_signal_new (QMI_DEVICE_SIGNAL_REMOVED,
                      G_OBJECT_CLASS_TYPE (G_OBJECT_CLASS (klass)),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL,
                      NULL,
                      NULL,
                      G_TYPE_NONE,
                      0);
}
