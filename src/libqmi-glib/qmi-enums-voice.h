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
 * Copyright (C) 2014-2020 Aleksander Morgado <aleksander@aleksander.es>
 * Copyright (C) 2019-2020 Alexander Couzens <lynxis@fe80.eu>
 */

#ifndef _LIBQMI_GLIB_QMI_ENUMS_VOICE_H_
#define _LIBQMI_GLIB_QMI_ENUMS_VOICE_H_

#if !defined (__LIBQMI_GLIB_H_INSIDE__) && !defined (LIBQMI_GLIB_COMPILATION)
#error "Only <libqmi-glib.h> can be included directly."
#endif

/**
 * SECTION: qmi-enums-voice
 * @title: VOICE enumerations and flags
 *
 * This section defines enumerations and flags used in the VOICE service
 * interface.
 */

/*****************************************************************************/
/* Helper enums for the 'QMI Voice All Call Status' indication */

/**
 * QmiVoiceCallState:
 * @QMI_VOICE_CALL_STATE_UNKNOWN: Unknown state.
 * @QMI_VOICE_CALL_STATE_ORIGINATION: Call is being originated.
 * @QMI_VOICE_CALL_STATE_INCOMING: Incoming call.
 * @QMI_VOICE_CALL_STATE_CONVERSATION: Call is in progress.
 * @QMI_VOICE_CALL_STATE_CC_IN_PROGRESS: Call is originating but waiting for call control to complete.
 * @QMI_VOICE_CALL_STATE_ALERTING: Alerting.
 * @QMI_VOICE_CALL_STATE_HOLD: On hold.
 * @QMI_VOICE_CALL_STATE_WAITING: Waiting.
 * @QMI_VOICE_CALL_STATE_DISCONNECTING: Disconnecting.
 * @QMI_VOICE_CALL_STATE_END: Call is finished.
 * @QMI_VOICE_CALL_STATE_SETUP: MT call is in setup state (3GPP).
 *
 * State of a call.
 *
 * Since: 1.14
 */
typedef enum {
    QMI_VOICE_CALL_STATE_UNKNOWN        = 0x00,
    QMI_VOICE_CALL_STATE_ORIGINATION    = 0x01,
    QMI_VOICE_CALL_STATE_INCOMING       = 0x02,
    QMI_VOICE_CALL_STATE_CONVERSATION   = 0x03,
    QMI_VOICE_CALL_STATE_CC_IN_PROGRESS = 0x04,
    QMI_VOICE_CALL_STATE_ALERTING       = 0x05,
    QMI_VOICE_CALL_STATE_HOLD           = 0x06,
    QMI_VOICE_CALL_STATE_WAITING        = 0x07,
    QMI_VOICE_CALL_STATE_DISCONNECTING  = 0x08,
    QMI_VOICE_CALL_STATE_END            = 0x09,
    QMI_VOICE_CALL_STATE_SETUP          = 0x0A,
} QmiVoiceCallState;

/**
 * qmi_voice_call_state_get_string:
 *
 * Since: 1.14
 */

/**
 * QmiVoiceCallType:
 * @QMI_VOICE_CALL_TYPE_VOICE: Voice call.
 * @QMI_VOICE_CALL_TYPE_VOICE_IP: VoIP call.
 * @QMI_VOICE_CALL_TYPE_OTAPA: OTAPA.
 * @QMI_VOICE_CALL_TYPE_NON_STD_OTASP: Non-standard OTASP.
 * @QMI_VOICE_CALL_TYPE_EMERGENCY: Emergency call.
 * @QMI_VOICE_CALL_TYPE_SUPS: Supplementary service.
 *
 * Type of a voice call.
 *
 * Since: 1.14
 */
typedef enum {
    QMI_VOICE_CALL_TYPE_VOICE         = 0x00,
    QMI_VOICE_CALL_TYPE_VOICE_IP      = 0x02,
    QMI_VOICE_CALL_TYPE_OTAPA         = 0x06,
    QMI_VOICE_CALL_TYPE_NON_STD_OTASP = 0x08,
    QMI_VOICE_CALL_TYPE_EMERGENCY     = 0x09,
    QMI_VOICE_CALL_TYPE_SUPS          = 0x0A,
} QmiVoiceCallType;

/**
 * qmi_voice_call_type_get_string:
 *
 * Since: 1.14
 */

/**
 * QmiVoiceCallDirection:
 * @QMI_VOICE_CALL_DIRECTION_UNKNOWN: Unknown.
 * @QMI_VOICE_CALL_DIRECTION_MO: Mobile-originated.
 * @QMI_VOICE_CALL_DIRECTION_MT: Mobile-terminated.
 *
 * Call direction.
 *
 * Since: 1.14
 */
typedef enum {
    QMI_VOICE_CALL_DIRECTION_UNKNOWN = 0x00,
    QMI_VOICE_CALL_DIRECTION_MO      = 0x01,
    QMI_VOICE_CALL_DIRECTION_MT      = 0x02,
} QmiVoiceCallDirection;

/**
 * qmi_voice_call_direction_get_string:
 *
 * Since: 1.14
 */

/**
 * QmiVoiceCallMode:
 * @QMI_VOICE_CALL_MODE_UNKNOWN: Unknown.
 * @QMI_VOICE_CALL_MODE_CDMA: CDMA.
 * @QMI_VOICE_CALL_MODE_GSM: GSM.
 * @QMI_VOICE_CALL_MODE_UMTS: UMTS.
 * @QMI_VOICE_CALL_MODE_LTE: LTE.
 *
 * Call mode.
 *
 * Since: 1.14
 */
typedef enum {
    QMI_VOICE_CALL_MODE_UNKNOWN = 0x00,
    QMI_VOICE_CALL_MODE_CDMA    = 0x01,
    QMI_VOICE_CALL_MODE_GSM     = 0x02,
    QMI_VOICE_CALL_MODE_UMTS    = 0x03,
    QMI_VOICE_CALL_MODE_LTE     = 0x04,
} QmiVoiceCallMode;

/**
 * qmi_voice_call_mode_get_string:
 *
 * Since: 1.14
 */

/**
 * QmiVoiceAls:
 * @QMI_VOICE_ALS_LINE_1: Line 1 (default).
 * @QMI_VOICE_ALS_LINE_2: Line 2.
 *
 * ALS line indicator.
 *
 * Since: 1.14
 */
typedef enum {
    QMI_VOICE_ALS_LINE_1 = 0x00,
    QMI_VOICE_ALS_LINE_2 = 0x01,
} QmiVoiceAls;

/**
 * qmi_voice_als_get_string:
 *
 * Since: 1.14
 */

/**
 * QmiVoicePresentation:
 * @QMI_VOICE_PRESENTATION_ALLOWED: Allowed presentation.
 * @QMI_VOICE_PRESENTATION_RESTRICTED: Restricted presentation.
 * @QMI_VOICE_PRESENTATION_UNAVAILABLE: Unavailable presentation.
 * @QMI_VOICE_PRESENTATION_PAYPHONE: Payphone presentation (3GPP only).
 *
 * Remote party number presentation indicator.
 *
 * Since: 1.14
 */
typedef enum {
    QMI_VOICE_PRESENTATION_ALLOWED     = 0x00,
    QMI_VOICE_PRESENTATION_RESTRICTED  = 0x01,
    QMI_VOICE_PRESENTATION_UNAVAILABLE = 0x02,
    QMI_VOICE_PRESENTATION_PAYPHONE    = 0x04,
} QmiVoicePresentation;

/**
 * qmi_voice_presentation_get_string:
 *
 * Since: 1.14
 */

/*****************************************************************************/
/* Helper enums for the 'QMI Voice Get Config' request/response */

/**
 * QmiVoiceTtyMode:
 * @QMI_VOICE_TTY_MODE_FULL: Full.
 * @QMI_VOICE_TTY_MODE_VCO: Voice carry over.
 * @QMI_VOICE_TTY_MODE_HCO: Hearing carry over.
 * @QMI_VOICE_TTY_MODE_OFF: Off.
 *
 * TTY mode.
 *
 * Since: 1.14
 */
typedef enum {
    QMI_VOICE_TTY_MODE_FULL = 0x00,
    QMI_VOICE_TTY_MODE_VCO  = 0x01,
    QMI_VOICE_TTY_MODE_HCO  = 0x02,
    QMI_VOICE_TTY_MODE_OFF  = 0x03,
} QmiVoiceTtyMode;

/**
 * qmi_voice_tty_mode_get_string:
 *
 * Since: 1.14
 */

/**
 * QmiVoiceServiceOption:
 * @QMI_VOICE_SERVICE_OPTION_WILD: Any service option.
 * @QMI_VOICE_SERVICE_OPTION_IS_96A: IS-96A.
 * @QMI_VOICE_SERVICE_OPTION_EVRC: EVRC.
 * @QMI_VOICE_SERVICE_OPTION_13K_IS733: IS733.
 * @QMI_VOICE_SERVICE_OPTION_SELECTABLE_MODE_VOCODER: Selectable mode vocoder.
 * @QMI_VOICE_SERVICE_OPTION_4GV_NARROW_BAND: 4GV narrowband.
 * @QMI_VOICE_SERVICE_OPTION_4GV_WIDE_BAND: 4GV wideband.
 * @QMI_VOICE_SERVICE_OPTION_13K: 13K.
 * @QMI_VOICE_SERVICE_OPTION_IS_96: IS-96.
 * @QMI_VOICE_SERVICE_OPTION_WVRC: WVRC.
 *
 * Service option.
 *
 * Since: 1.14
 */
typedef enum {
    QMI_VOICE_SERVICE_OPTION_WILD                    = 0x0000,
    QMI_VOICE_SERVICE_OPTION_IS_96A                  = 0x0001,
    QMI_VOICE_SERVICE_OPTION_EVRC                    = 0x0003,
    QMI_VOICE_SERVICE_OPTION_13K_IS733               = 0x0011,
    QMI_VOICE_SERVICE_OPTION_SELECTABLE_MODE_VOCODER = 0x0038,
    QMI_VOICE_SERVICE_OPTION_4GV_NARROW_BAND         = 0x0044,
    QMI_VOICE_SERVICE_OPTION_4GV_WIDE_BAND           = 0x0046,
    QMI_VOICE_SERVICE_OPTION_13K                     = 0x8000,
    QMI_VOICE_SERVICE_OPTION_IS_96                   = 0x8001,
    QMI_VOICE_SERVICE_OPTION_WVRC                    = 0x8023,
} QmiVoiceServiceOption;

/**
 * qmi_voice_service_option_get_string:
 *
 * Since: 1.14
 */

/**
 * QmiVoiceWcdmaAmrStatus:
 * @QMI_VOICE_WCDMA_AMR_STATUS_NOT_SUPPORTED: Not supported.
 * @QMI_VOICE_WCDMA_AMR_STATUS_WCDMA_AMR_WB: WCDMA AMR wideband.
 * @QMI_VOICE_WCDMA_AMR_STATUS_GSM_HR_AMR: GSM half-rate AMR.
 * @QMI_VOICE_WCDMA_AMR_STATUS_GSM_AMR_WB: GSM AMR wideband.
 * @QMI_VOICE_WCDMA_AMR_STATUS_GSM_AMR_NB: GSM AMR narrowband.
 *
 * WCDMA AMR status.
 *
 * Since: 1.14
 */
typedef enum {
    QMI_VOICE_WCDMA_AMR_STATUS_NOT_SUPPORTED = 1 << 0,
    QMI_VOICE_WCDMA_AMR_STATUS_WCDMA_AMR_WB  = 1 << 1,
    QMI_VOICE_WCDMA_AMR_STATUS_GSM_HR_AMR    = 1 << 2,
    QMI_VOICE_WCDMA_AMR_STATUS_GSM_AMR_WB    = 1 << 3,
    QMI_VOICE_WCDMA_AMR_STATUS_GSM_AMR_NB    = 1 << 4,
} QmiVoiceWcdmaAmrStatus;

/**
 * qmi_voice_wcdma_amr_status_build_string_from_mask:
 *
 * Since: 1.14
 */

/**
 * QmiVoicePrivacy:
 * @QMI_VOICE_PRIVACY_STANDARD: Standard.
 * @QMI_VOICE_PRIVACY_ENHANCED: Enhanced.
 *
 * Voice privacy.
 *
 * Since: 1.14
 */
typedef enum {
    QMI_VOICE_PRIVACY_STANDARD = 0x00,
    QMI_VOICE_PRIVACY_ENHANCED = 0x01,
} QmiVoicePrivacy;

/**
 * qmi_voice_privacy_get_string:
 *
 * Since: 1.14
 */

/**
 * QmiVoiceDomain:
 * @QMI_VOICE_DOMAIN_CS_ONLY: CS-only.
 * @QMI_VOICE_DOMAIN_PS_ONLY: PS-only.
 * @QMI_VOICE_DOMAIN_CS_PREFERRED: CS preferred, PS secondary.
 * @QMI_VOICE_DOMAIN_PS_PREFERRED: PS preferred, CS secondary.
 *
 * Voice domain preference.
 *
 * Since: 1.14
 */
typedef enum {
    QMI_VOICE_DOMAIN_CS_ONLY      = 0x00,
    QMI_VOICE_DOMAIN_PS_ONLY      = 0x01,
    QMI_VOICE_DOMAIN_CS_PREFERRED = 0x02,
    QMI_VOICE_DOMAIN_PS_PREFERRED = 0x03,
} QmiVoiceDomain;

/**
 * qmi_voice_domain_get_string:
 *
 * Since: 1.14
 */

/**
 * QmiVoiceUserAction:
 * @QMI_VOICE_USER_ACTION_UNKNOWN: Unknown user action.
 * @QMI_VOICE_USER_ACTION_NOT_REQUIRED: User is not required to respond to the USSD code.
 * @QMI_VOICE_USER_ACTION_REQUIRED: User is required to respond to the USSD code.
 *
 * User action type.
 */
typedef enum {
    QMI_VOICE_USER_ACTION_UNKNOWN      = 0x00,
    QMI_VOICE_USER_ACTION_NOT_REQUIRED = 0x01,
    QMI_VOICE_USER_ACTION_REQUIRED     = 0x02,
} QmiVoiceUserAction;

/**
 * qmi_voice_user_action_get_string:
 *
 * Since: 1.26
 */

/**
 * QmiVoiceUssDataCodingScheme:
 * @QMI_VOICE_USS_DATA_CODING_SCHEME_UNKNOWN: Unknown.
 * @QMI_VOICE_USS_DATA_CODING_SCHEME_ASCII: ASCII coding scheme.
 * @QMI_VOICE_USS_DATA_CODING_SCHEME_8BIT: 8-bit coding scheme.
 * @QMI_VOICE_USS_DATA_CODING_SCHEME_UCS2: UCS2.
 *
 * Data Coding Scheme used in USSD operations.
 *
 * Since: 1.26
 */
typedef enum {
    QMI_VOICE_USS_DATA_CODING_SCHEME_UNKNOWN = 0x00,
    QMI_VOICE_USS_DATA_CODING_SCHEME_ASCII   = 0x01,
    QMI_VOICE_USS_DATA_CODING_SCHEME_8BIT    = 0x02,
    QMI_VOICE_USS_DATA_CODING_SCHEME_UCS2    = 0x03,
} QmiVoiceUssDataCodingScheme;

/**
 * qmi_voice_uss_data_coding_scheme_get_string:
 *
 * Since: 1.26
 */

/**
 * QmiVoiceAlphaDataCodingScheme:
 * @QMI_VOICE_ALPHA_DATA_CODING_SCHEME_GSM: SMS default 7-bit coded alphabet.
 * @QMI_VOICE_ALPHA_DATA_CODING_SCHEME_UCS2: UCS2.
 *
 * Alpha Coding Scheme.
 *
 * Since: 1.26
 */
typedef enum {
    QMI_VOICE_ALPHA_DATA_CODING_SCHEME_GSM  = 0x01,
    QMI_VOICE_ALPHA_DATA_CODING_SCHEME_UCS2 = 0x02,
} QmiVoiceAlphaDataCodingScheme;

/**
 * qmi_voice_alpha_data_coding_scheme_get_string:
 *
 * Since: 1.26
 */

/**
 * QmiVoiceCallEndReason:
 * @QMI_VOICE_CALL_END_REASON_OFFLINE: Device is offline.
 * @QMI_VOICE_CALL_END_REASON_CDMA_LOCK: (CDMA) Phone is CDMA locked.
 * @QMI_VOICE_CALL_END_REASON_NO_SERVICE: Device has no service.
 * @QMI_VOICE_CALL_END_REASON_FADE: Fade.
 * @QMI_VOICE_CALL_END_REASON_INTERCEPT: (CDMA) Received intercept from the BS.
 * @QMI_VOICE_CALL_END_REASON_REORDER: (CDMA) Received reorder from the BS.
 * @QMI_VOICE_CALL_END_REASON_RELEASE_NORMAL: Received release from the BS.
 * @QMI_VOICE_CALL_END_REASON_RELEASE_SO_REJECT: (CDMA) Received release from the BS.
 * @QMI_VOICE_CALL_END_REASON_INCOMING_CALL: (CDMA) Received incoming call from the BS.
 * @QMI_VOICE_CALL_END_REASON_ALERT_STOP: (CDMA) Received alert stop from the BS.
 * @QMI_VOICE_CALL_END_REASON_CLIENT_END: Client ended the call.
 * @QMI_VOICE_CALL_END_REASON_ACTIVATION: (CDMA) Received end activation.
 * @QMI_VOICE_CALL_END_REASON_MC_ABORT: (CDMA) MC aborted the origination/conversation.
 * @QMI_VOICE_CALL_END_REASON_MAX_ACCESS_PROBE: (CDMA) Maximum access probes transmitted.
 * @QMI_VOICE_CALL_END_REASON_PSIST_N: (CDMA) Persistence test failure.
 * @QMI_VOICE_CALL_END_REASON_UIM_NOT_PRESENT: R-UIM not present.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_ATTEMPT_IN_PROGRESS: Access attempt in progress.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_FAILURE: Access failure.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_RETRY_ORDER: (CDMA) Retry order.
 * @QMI_VOICE_CALL_END_REASON_CCS_NOT_SUPPORTED_BY_BS: (CDMA) Concurrent service not supported by the BS.
 * @QMI_VOICE_CALL_END_REASON_NO_RESPONSE_FROM_BS: (CDMA) No response received from the BS.
 * @QMI_VOICE_CALL_END_REASON_REJECTED_BY_BS: (CDMA) Rejected by the BS.
 * @QMI_VOICE_CALL_END_REASON_INCOMPATIBLE: (CDMA) Concurrent services requested are incompatible.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_BLOCK: (CDMA) Access blocked by the BS.
 * @QMI_VOICE_CALL_END_REASON_ALREADY_IN_TC: Already in TC.
 * @QMI_VOICE_CALL_END_REASON_EMERGENCY_FLASHED: (CDMA) Emergency call is flashed over this call.
 * @QMI_VOICE_CALL_END_REASON_USER_CALL_ORIGINATED_DURING_GPS: Call originated during GPS.
 * @QMI_VOICE_CALL_END_REASON_USER_CALL_ORIGINATED_DURING_SMS: Call originated during SMS.
 * @QMI_VOICE_CALL_END_REASON_USER_CALL_ORIGINATED_DURING_DATA: Call originated during data.
 * @QMI_VOICE_CALL_END_REASON_REDIRECTION_OR_HANDOFF: Redirection or handoff.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_BLOCK_ALL: Access blocked by BS for all.
 * @QMI_VOICE_CALL_END_REASON_OTASP_SPC_ERR: OTASP SPC error indication.
 * @QMI_VOICE_CALL_END_REASON_IS707B_MAX_ACCESS_PROBES: Maximum access probes for IS-707B call.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_FAILURE_REJECT_ORDER: Base station reject order.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_FAILURE_RETRY_ORDER: Base station retry order.
 * @QMI_VOICE_CALL_END_REASON_TIMEOUT_T42: Timer T42 expired.
 * @QMI_VOICE_CALL_END_REASON_TIMEOUT_T40: Timer T40 expired.
 * @QMI_VOICE_CALL_END_REASON_SERVICE_INIT_FAILURE: Service initialization failure.
 * @QMI_VOICE_CALL_END_REASON_TIMEOUT_T50: Timer T50 expired.
 * @QMI_VOICE_CALL_END_REASON_TIMEOUT_T51: Timer T51 expired.
 * @QMI_VOICE_CALL_END_REASON_RL_ACK_TIMEOUT: Acknowledgement timeout due to 12 retransmissions.
 * @QMI_VOICE_CALL_END_REASON_BAD_FORWARD_LINK: Bad forward link or timer T5M expired.
 * @QMI_VOICE_CALL_END_REASON_TRM_REQUEST_FAILED: Transceiver Resource Manager request failed.
 * @QMI_VOICE_CALL_END_REASON_TIMEOUT_T41: Timer T41 expired.
 * @QMI_VOICE_CALL_END_REASON_INCOMING_REJECTED: (GSM/WCDMA) Client rejected incoming call.
 * @QMI_VOICE_CALL_END_REASON_SETUP_REJECTED: (GSM/WCDMA) Client rejected a setup indication.
 * @QMI_VOICE_CALL_END_REASON_NETWORK_END: (GSM/WCDMA) Network ended the call.
 * @QMI_VOICE_CALL_END_REASON_NO_FUNDS: (GSM/WCDMA) No funds.
 * @QMI_VOICE_CALL_END_REASON_NO_GW_SERVICE: (GSM/WCDMA) Device has no service.
 * @QMI_VOICE_CALL_END_REASON_NO_CDMA_SERVICE: (CDMA) Device has no service.
 * @QMI_VOICE_CALL_END_REASON_NO_FULL_SERVICE: Full service is unavailable.
 * @QMI_VOICE_CALL_END_REASON_MAX_PS_CALLS: No resources available to handle a new MO/MT PS call.
 * @QMI_VOICE_CALL_END_REASON_UNKNOWN_SUBSCRIBER: Unknown subscriber.
 * @QMI_VOICE_CALL_END_REASON_ILLEGAL_SUBSCRIBER: Illegal subscriber.
 * @QMI_VOICE_CALL_END_REASON_BEARER_SERVICE_NOT_PROVISIONED: Bearer service not provisioned.
 * @QMI_VOICE_CALL_END_REASON_TELE_SERVICE_NOT_PROVISIONED: Tele service not provisioned.
 * @QMI_VOICE_CALL_END_REASON_ILLEGAL_EQUIPMENT: Illegal equipment.
 * @QMI_VOICE_CALL_END_REASON_CALL_BARRED: Call barred.
 * @QMI_VOICE_CALL_END_REASON_ILLEGAL_SS_OPERATION: Illegal SS operation.
 * @QMI_VOICE_CALL_END_REASON_SS_ERROR_STATUS: Supplementary service error status.
 * @QMI_VOICE_CALL_END_REASON_SS_NOT_AVAILABLE: Supplementary service not available.
 * @QMI_VOICE_CALL_END_REASON_SS_SUBSCRIPTION_VIOLATION: Supplementary service subscription violation.
 * @QMI_VOICE_CALL_END_REASON_SS_INCOMPATIBILITY: Supplementary service incompatibility.
 * @QMI_VOICE_CALL_END_REASON_FACILITY_NOT_SUPPORTED: Facility not supported.
 * @QMI_VOICE_CALL_END_REASON_ABSENT_SUBSCRIBER: Absent subscriber.
 * @QMI_VOICE_CALL_END_REASON_SHORT_TERM_DENIAL: Short term denial.
 * @QMI_VOICE_CALL_END_REASON_LONG_TERM_DENIAL: Long term denial.
 * @QMI_VOICE_CALL_END_REASON_SYSTEM_FAILURE: System failure.
 * @QMI_VOICE_CALL_END_REASON_DATA_MISSING: Data missing.
 * @QMI_VOICE_CALL_END_REASON_UNEXPECTED_DATA_VALUE: Unexpected data value.
 * @QMI_VOICE_CALL_END_REASON_PASSWORD_REGISTRATION_FAILURE: Password registration failure.
 * @QMI_VOICE_CALL_END_REASON_NEGATIVE_PASSWORD_CHECK: Negative password check.
 * @QMI_VOICE_CALL_END_REASON_NUM_OF_PASSWORD_ATTEMPTS_VIOLATION: Number of password attempts violation.
 * @QMI_VOICE_CALL_END_REASON_POSITION_METHOD_FAILURE: Position method failure.
 * @QMI_VOICE_CALL_END_REASON_UNKNOWN_ALPHABET: Unknown alphabet.
 * @QMI_VOICE_CALL_END_REASON_USSD_BUSY: USSD busy.
 * @QMI_VOICE_CALL_END_REASON_REJECTED_BY_USER: Rejected by user.
 * @QMI_VOICE_CALL_END_REASON_REJECTED_BY_NETWORK: Rejected by network.
 * @QMI_VOICE_CALL_END_REASON_DEFLECTION_TO_SERVED_SUBSCRIBER: Deflection to served subscriber.
 * @QMI_VOICE_CALL_END_REASON_SPECIAL_SERVICE_CODE: Special service codde.
 * @QMI_VOICE_CALL_END_REASON_INVALID_DEFLECTED_TO_NUMBER: Invalid deflected to number.
 * @QMI_VOICE_CALL_END_REASON_MULTIPARTY_PARTICIPANTS_EXCEEDED: Multiparty participants exceeded.
 * @QMI_VOICE_CALL_END_REASON_RESOURCES_NOT_AVAILABLE: Resources not available.
 * @QMI_VOICE_CALL_END_REASON_UNASSIGNED_NUMBER: Unassigned number.
 * @QMI_VOICE_CALL_END_REASON_NO_ROUTE_TO_DESTINATION: No route to destination.
 * @QMI_VOICE_CALL_END_REASON_CHANNEL_UNACCEPTABLE: Channel unacceptable.
 * @QMI_VOICE_CALL_END_REASON_OPERATOR_DETERMINED_BARRING: Operator determined barring.
 * @QMI_VOICE_CALL_END_REASON_NORMAL_CALL_CLEARING: Normal call clearing.
 * @QMI_VOICE_CALL_END_REASON_USER_BUSY: User busy.
 * @QMI_VOICE_CALL_END_REASON_NO_USER_RESPONDING: No user responding.
 * @QMI_VOICE_CALL_END_REASON_USER_ALERTING_NO_ANSWER: User alerting no answer.
 * @QMI_VOICE_CALL_END_REASON_CALL_REJECTED: Call rejected.
 * @QMI_VOICE_CALL_END_REASON_NUMBER_CHANGED: Number changed.
 * @QMI_VOICE_CALL_END_REASON_PREEMPTION: Preemption.
 * @QMI_VOICE_CALL_END_REASON_DESTINATION_OUT_OF_ORDER: Destination out of order.
 * @QMI_VOICE_CALL_END_REASON_INVALID_NUMBER_FORMAT: Invalid number format.
 * @QMI_VOICE_CALL_END_REASON_FACILITY_REJECTED: Facility rejected.
 * @QMI_VOICE_CALL_END_REASON_RESPONSE_TO_STATUS_ENQUIRY: Response to status enquiry.
 * @QMI_VOICE_CALL_END_REASON_NORMAL_UNSPECIFIED: Normal unspecified.
 * @QMI_VOICE_CALL_END_REASON_NO_CIRCUIT_OR_CHANNEL_AVAILABLE: No circuit or channel available.
 * @QMI_VOICE_CALL_END_REASON_NETWORK_OUT_OF_ORDER: Network out of order.
 * @QMI_VOICE_CALL_END_REASON_TEMPORARY_FAILURE: Temporary failure.
 * @QMI_VOICE_CALL_END_REASON_SWITCHING_EQUIPMENT_CONGESTION: Switching equipment congestion.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_INFORMATION_DISCARDED: Access information discarded.
 * @QMI_VOICE_CALL_END_REASON_REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE: Requested circuit or channel not available.
 * @QMI_VOICE_CALL_END_REASON_RESOURCES_UNAVAILABLE_OR_UNSPECIFIED: Resources unavailable or unspecified.
 * @QMI_VOICE_CALL_END_REASON_QOS_UNAVAILABLE: QoS unavailable.
 * @QMI_VOICE_CALL_END_REASON_REQUESTED_FACILITY_NOT_SUBSCRIBED: Requested facility not subscribed.
 * @QMI_VOICE_CALL_END_REASON_INCOMING_CALLS_BARRED_WITHIN_CUG: Incoming calls barred withing closed user group.
 * @QMI_VOICE_CALL_END_REASON_BEARER_CAPABILITY_NOT_AUTH: Bearer capability not auth.
 * @QMI_VOICE_CALL_END_REASON_BEARER_CAPABILITY_UNAVAILABLE: Bearer capability unavailable.
 * @QMI_VOICE_CALL_END_REASON_SERVICE_OPTION_NOT_AVAILABLE: Service option not available.
 * @QMI_VOICE_CALL_END_REASON_ACM_LIMIT_EXCEEDED: ACM limit exceeded.
 * @QMI_VOICE_CALL_END_REASON_BEARER_SERVICE_NOT_IMPLEMENTED: Bearer service not implemented.
 * @QMI_VOICE_CALL_END_REASON_REQUESTED_FACILITY_NOT_IMPLEMENTED: Requested facility not implemented.
 * @QMI_VOICE_CALL_END_REASON_ONLY_DIGITAL_INFORMATION_BEARER_AVAILABLE: Only digital information bearer available.
 * @QMI_VOICE_CALL_END_REASON_SERVICE_OR_OPTION_NOT_IMPLEMENTED: Service or option not implemented.
 * @QMI_VOICE_CALL_END_REASON_INVALID_TRANSACTION_IDENTIFIER: Invalid transaction identifier.
 * @QMI_VOICE_CALL_END_REASON_USER_NOT_MEMBER_OF_CUG: User not member of closed user group.
 * @QMI_VOICE_CALL_END_REASON_INCOMPATIBLE_DESTINATION: Incompatible destination.
 * @QMI_VOICE_CALL_END_REASON_INVALID_TRANSIT_NETWORK_SELECTION: Invalid transit network selection.
 * @QMI_VOICE_CALL_END_REASON_SEMANTICALLY_INCORRECT_MESSAGE: Semantically incorrect message.
 * @QMI_VOICE_CALL_END_REASON_INVALID_MANDATORY_INFORMATION: Invalid mandatory information.
 * @QMI_VOICE_CALL_END_REASON_MESSAGE_TYPE_NOT_IMPLEMENTED: Message type not implemented.
 * @QMI_VOICE_CALL_END_REASON_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE: Message type not compatible with protocol state.
 * @QMI_VOICE_CALL_END_REASON_INFORMATION_ELEMENT_NON_EXISTENT: Information element non existent.
 * @QMI_VOICE_CALL_END_REASON_CONDITIONAL_IE_ERROR: IE error.
 * @QMI_VOICE_CALL_END_REASON_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE: Message not compatible with protocol state.
 * @QMI_VOICE_CALL_END_REASON_RECOVERY_ON_TIMER_EXPIRED: Recovery on timer expired.
 * @QMI_VOICE_CALL_END_REASON_PROTOCOL_ERROR_UNSPECIFIED: Protocol error unspecified.
 * @QMI_VOICE_CALL_END_REASON_INTERWORKING_UNSPECIFIED: Interworking unspecified.
 * @QMI_VOICE_CALL_END_REASON_OUTGOING_CALLS_BARRED_WITHIN_CUG: Outgoing calls barred within closed user group.
 * @QMI_VOICE_CALL_END_REASON_NO_CUG_SELECTION: No closed user group selection.
 * @QMI_VOICE_CALL_END_REASON_UNKNOWN_CUG_INDEX: Unknown closed user group index.
 * @QMI_VOICE_CALL_END_REASON_CUG_INDEX_INCOMPATIBLE: Closed user group index incompatible.
 * @QMI_VOICE_CALL_END_REASON_CUG_CALL_FAILURE_UNSPECIFIED: Closed user group call failure unspecified.
 * @QMI_VOICE_CALL_END_REASON_CLIR_NOT_SUBSCRIBED: Calling line identification restriction not subscribed.
 * @QMI_VOICE_CALL_END_REASON_CCBS_POSSIBLE: Completion of communications to busy subscriber possible.
 * @QMI_VOICE_CALL_END_REASON_CCBS_NOT_POSSIBLE: Completion of communications to busy subscriber not possible.
 * @QMI_VOICE_CALL_END_REASON_IMSI_UNKNOWN_IN_HLR: IMSI unknown in HLR.
 * @QMI_VOICE_CALL_END_REASON_ILLEGAL_MS: Illegal MS.
 * @QMI_VOICE_CALL_END_REASON_IMSI_UNKNOWN_IN_VLR: IMSI unknown in VLR.
 * @QMI_VOICE_CALL_END_REASON_IMEI_NOT_ACCEPTED: IMEI not accepted.
 * @QMI_VOICE_CALL_END_REASON_ILLEGAL_ME: Illegal ME.
 * @QMI_VOICE_CALL_END_REASON_PLMN_NOT_ALLOWED: PLMN not allowed.
 * @QMI_VOICE_CALL_END_REASON_LOCATION_AREA_NOT_ALLOWED: Location area not allowed.
 * @QMI_VOICE_CALL_END_REASON_ROAMING_NOT_ALLOWED_IN_THIS_LOCATION_AREA: Roaming not allowed in this location area.
 * @QMI_VOICE_CALL_END_REASON_NO_SUITABLE_CELLS_IN_LOCATION_AREA: No suitable cells in location area.
 * @QMI_VOICE_CALL_END_REASON_NETWORK_FAILURE: Network failure.
 * @QMI_VOICE_CALL_END_REASON_MAC_FAILURE: MAC failure.
 * @QMI_VOICE_CALL_END_REASON_SYNCH_FAILURE: Synch failure.
 * @QMI_VOICE_CALL_END_REASON_NETWORK_CONGESTION: Network contestion.
 * @QMI_VOICE_CALL_END_REASON_GSM_AUTHENTICATION_UNACCEPTABLE: GSM authentication unacceptable.
 * @QMI_VOICE_CALL_END_REASON_SERVICE_NOT_SUBSCRIBED: Service not subscribed.
 * @QMI_VOICE_CALL_END_REASON_SERVICE_TEMPORARILY_OUT_OF_ORDER: Service temporarily out of order.
 * @QMI_VOICE_CALL_END_REASON_CALL_CANNOT_BE_IDENTIFIED: Call cannot be identified.
 * @QMI_VOICE_CALL_END_REASON_INCORRECT_SEMANTICS_IN_MESSAGE: Incorrect semantics in message.
 * @QMI_VOICE_CALL_END_REASON_MANDATORY_INFORMATION_INVALID: Mandatory information invalid.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_FAILURE: Access stratum failure.
 * @QMI_VOICE_CALL_END_REASON_INVALID_SIM: Invalid SIM.
 * @QMI_VOICE_CALL_END_REASON_WRONG_STATE: Wrong state.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_CLASS_BLOCKED: Access class blocked.
 * @QMI_VOICE_CALL_END_REASON_NO_RESOURCES: No resources.
 * @QMI_VOICE_CALL_END_REASON_INVALID_USER_DATA: Invalid user data.
 * @QMI_VOICE_CALL_END_REASON_TIMER_T3230_EXPIRED: Timer T3230 expired.
 * @QMI_VOICE_CALL_END_REASON_NO_CELL_AVAILABLE: No cell available.
 * @QMI_VOICE_CALL_END_REASON_ABORT_MESSAGE_RECEIVED: Abort message received.
 * @QMI_VOICE_CALL_END_REASON_RADIO_LINK_LOST: Radio link lost.
 * @QMI_VOICE_CALL_END_REASON_TIMER_T303_EXPIRED: Timer T303 expired.
 * @QMI_VOICE_CALL_END_REASON_CNM_MM_RELEASE_PENDING: CNM MM release pending.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_REJECT_RR_RELEASE_INDICATION: Access stratum reject, RR release indication.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_REJECT_RR_RANDOM_ACCESS_FAILURE: Access stratum reject, RR random access failure.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_REJECT_RRC_RELEASE_INDICATION: Access stratum reject, RRC release indication.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_REJECT_RRC_CLOSE_SESSION_INDICATION: Access stratum reject, RRC close session indication.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_REJECT_RRC_OPEN_SESSION_FAILURE: Access stratum reject, RRC open session failure.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_REJECT_LOW_LEVEL_FAILURE: Access stratum reject, low level failure.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_REJECT_LOW_LEVEL_FAILURE_REDIAL_NOT_ALLOWED: Access stratum reject, low level failure redial not allowed.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_REJECT_LOW_LEVEL_IMMEDIATE_RETRY: Access stratum reject, low level immediate retry.
 * @QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_REJECT_ABORT_RADIO_UNAVAILABLE: Access stratum reject, abort radio unavailable.
 * @QMI_VOICE_CALL_END_REASON_SERVICE_OPTION_NOT_SUPPORTED: Service option not supported.
 * @QMI_VOICE_CALL_END_REASON_BAD_REQUEST_WAIT_INVITE: Bad request wait invite.
 * @QMI_VOICE_CALL_END_REASON_BAD_REQUEST_WAIT_REINVITE: Bad request wait reinvite.
 * @QMI_VOICE_CALL_END_REASON_INVALID_REMOTE_URI: Invalid remote URI.
 * @QMI_VOICE_CALL_END_REASON_REMOTE_UNSUPPORTED_MEDIA_TYPE: Remote unsupported media type.
 * @QMI_VOICE_CALL_END_REASON_PEER_NOT_REACHABLE: Peer not reachable.
 * @QMI_VOICE_CALL_END_REASON_NETWORK_NO_RESPONSE_TIMEOUT: Network no response, timeout.
 * @QMI_VOICE_CALL_END_REASON_NETWORK_NO_RESPONSE_HOLD_FAILURE: Network no response, hold failure.
 * @QMI_VOICE_CALL_END_REASON_DATA_CONNECTION_LOST: Data connection lost.
 * @QMI_VOICE_CALL_END_REASON_UPGRADE_DOWNGRADE_REJECTED: Upgrade/downgrade rejected.
 * @QMI_VOICE_CALL_END_REASON_SIP_403_FORBIDDEN: SIP 403 forbidden.
 * @QMI_VOICE_CALL_END_REASON_NO_NETWORK_RESPONSE: No network response.
 * @QMI_VOICE_CALL_END_REASON_UPGRADE_DOWNGRADE_FAILED: Upgrade/downgrade failed.
 * @QMI_VOICE_CALL_END_REASON_UPGRADE_DOWNGRADE_CANCELLED: Upgrade/downgrade cancelled.
 * @QMI_VOICE_CALL_END_REASON_SSAC_REJECT: Service specific access control reject.
 * @QMI_VOICE_CALL_END_REASON_THERMAL_EMERGENCY: Thermal emergency.
 * @QMI_VOICE_CALL_END_REASON_1XCSFB_SOFT_FAILURE: 1xCSFG soft failure.
 * @QMI_VOICE_CALL_END_REASON_1XCSFB_HARD_FAILURE: 1xCSFG hard failure.
 *
 * Possible call end reasons resulting from a voice call or supplementary
 * service connection being terminated.
 *
 * Since: 1.26
 */
typedef enum {
    QMI_VOICE_CALL_END_REASON_OFFLINE                            = 0,
    QMI_VOICE_CALL_END_REASON_CDMA_LOCK                          = 20,
    QMI_VOICE_CALL_END_REASON_NO_SERVICE                         = 21,
    QMI_VOICE_CALL_END_REASON_FADE                               = 22,
    QMI_VOICE_CALL_END_REASON_INTERCEPT                          = 23,
    QMI_VOICE_CALL_END_REASON_REORDER                            = 24,
    QMI_VOICE_CALL_END_REASON_RELEASE_NORMAL                     = 25,
    QMI_VOICE_CALL_END_REASON_RELEASE_SO_REJECT                  = 26,
    QMI_VOICE_CALL_END_REASON_INCOMING_CALL                      = 27,
    QMI_VOICE_CALL_END_REASON_ALERT_STOP                         = 28,
    QMI_VOICE_CALL_END_REASON_CLIENT_END                         = 29,
    QMI_VOICE_CALL_END_REASON_ACTIVATION                         = 30,
    QMI_VOICE_CALL_END_REASON_MC_ABORT                           = 31,
    QMI_VOICE_CALL_END_REASON_MAX_ACCESS_PROBE                   = 32,
    QMI_VOICE_CALL_END_REASON_PSIST_N                            = 33,
    QMI_VOICE_CALL_END_REASON_UIM_NOT_PRESENT                    = 34,
    QMI_VOICE_CALL_END_REASON_ACCESS_ATTEMPT_IN_PROGRESS         = 35,
    QMI_VOICE_CALL_END_REASON_ACCESS_FAILURE                     = 36,
    QMI_VOICE_CALL_END_REASON_ACCESS_RETRY_ORDER                 = 37,
    QMI_VOICE_CALL_END_REASON_CCS_NOT_SUPPORTED_BY_BS            = 38,
    QMI_VOICE_CALL_END_REASON_NO_RESPONSE_FROM_BS                = 39,
    QMI_VOICE_CALL_END_REASON_REJECTED_BY_BS                     = 40,
    QMI_VOICE_CALL_END_REASON_INCOMPATIBLE                       = 41,
    QMI_VOICE_CALL_END_REASON_ACCESS_BLOCK                       = 42,
    QMI_VOICE_CALL_END_REASON_ALREADY_IN_TC                      = 43,
    QMI_VOICE_CALL_END_REASON_EMERGENCY_FLASHED                  = 44,
    QMI_VOICE_CALL_END_REASON_USER_CALL_ORIGINATED_DURING_GPS    = 45,
    QMI_VOICE_CALL_END_REASON_USER_CALL_ORIGINATED_DURING_SMS    = 46,
    QMI_VOICE_CALL_END_REASON_USER_CALL_ORIGINATED_DURING_DATA   = 47,
    QMI_VOICE_CALL_END_REASON_REDIRECTION_OR_HANDOFF             = 48,
    QMI_VOICE_CALL_END_REASON_ACCESS_BLOCK_ALL                   = 49,
    QMI_VOICE_CALL_END_REASON_OTASP_SPC_ERR                      = 50,
    QMI_VOICE_CALL_END_REASON_IS707B_MAX_ACCESS_PROBES           = 51,
    QMI_VOICE_CALL_END_REASON_ACCESS_FAILURE_REJECT_ORDER        = 52,
    QMI_VOICE_CALL_END_REASON_ACCESS_FAILURE_RETRY_ORDER         = 53,
    QMI_VOICE_CALL_END_REASON_TIMEOUT_T42                        = 54,
    QMI_VOICE_CALL_END_REASON_TIMEOUT_T40                        = 55,
    QMI_VOICE_CALL_END_REASON_SERVICE_INIT_FAILURE               = 56,
    QMI_VOICE_CALL_END_REASON_TIMEOUT_T50                        = 57,
    QMI_VOICE_CALL_END_REASON_TIMEOUT_T51                        = 58,
    QMI_VOICE_CALL_END_REASON_RL_ACK_TIMEOUT                     = 59,
    QMI_VOICE_CALL_END_REASON_BAD_FORWARD_LINK                   = 60,
    QMI_VOICE_CALL_END_REASON_TRM_REQUEST_FAILED                 = 61,
    QMI_VOICE_CALL_END_REASON_TIMEOUT_T41                        = 62,
    QMI_VOICE_CALL_END_REASON_INCOMING_REJECTED                  = 102,
    QMI_VOICE_CALL_END_REASON_SETUP_REJECTED                     = 103,
    QMI_VOICE_CALL_END_REASON_NETWORK_END                        = 104,
    QMI_VOICE_CALL_END_REASON_NO_FUNDS                           = 105,
    QMI_VOICE_CALL_END_REASON_NO_GW_SERVICE                      = 106,
    QMI_VOICE_CALL_END_REASON_NO_CDMA_SERVICE                    = 107,
    QMI_VOICE_CALL_END_REASON_NO_FULL_SERVICE                    = 108,
    QMI_VOICE_CALL_END_REASON_MAX_PS_CALLS                       = 109,
    QMI_VOICE_CALL_END_REASON_UNKNOWN_SUBSCRIBER                 = 110,
    QMI_VOICE_CALL_END_REASON_ILLEGAL_SUBSCRIBER                 = 111,
    QMI_VOICE_CALL_END_REASON_BEARER_SERVICE_NOT_PROVISIONED     = 112,
    QMI_VOICE_CALL_END_REASON_TELE_SERVICE_NOT_PROVISIONED       = 113,
    QMI_VOICE_CALL_END_REASON_ILLEGAL_EQUIPMENT                  = 114,
    QMI_VOICE_CALL_END_REASON_CALL_BARRED                        = 115,
    QMI_VOICE_CALL_END_REASON_ILLEGAL_SS_OPERATION               = 116,
    QMI_VOICE_CALL_END_REASON_SS_ERROR_STATUS                    = 117,
    QMI_VOICE_CALL_END_REASON_SS_NOT_AVAILABLE                   = 118,
    QMI_VOICE_CALL_END_REASON_SS_SUBSCRIPTION_VIOLATION          = 119,
    QMI_VOICE_CALL_END_REASON_SS_INCOMPATIBILITY                 = 120,
    QMI_VOICE_CALL_END_REASON_FACILITY_NOT_SUPPORTED             = 121,
    QMI_VOICE_CALL_END_REASON_ABSENT_SUBSCRIBER                  = 122,
    QMI_VOICE_CALL_END_REASON_SHORT_TERM_DENIAL                  = 123,
    QMI_VOICE_CALL_END_REASON_LONG_TERM_DENIAL                   = 124,
    QMI_VOICE_CALL_END_REASON_SYSTEM_FAILURE                     = 125,
    QMI_VOICE_CALL_END_REASON_DATA_MISSING                       = 126,
    QMI_VOICE_CALL_END_REASON_UNEXPECTED_DATA_VALUE              = 127,
    QMI_VOICE_CALL_END_REASON_PASSWORD_REGISTRATION_FAILURE      = 128,
    QMI_VOICE_CALL_END_REASON_NEGATIVE_PASSWORD_CHECK            = 129,
    QMI_VOICE_CALL_END_REASON_NUM_OF_PASSWORD_ATTEMPTS_VIOLATION = 130,
    QMI_VOICE_CALL_END_REASON_POSITION_METHOD_FAILURE            = 131,
    QMI_VOICE_CALL_END_REASON_UNKNOWN_ALPHABET                   = 132,
    QMI_VOICE_CALL_END_REASON_USSD_BUSY                          = 133,
    QMI_VOICE_CALL_END_REASON_REJECTED_BY_USER                   = 134,
    QMI_VOICE_CALL_END_REASON_REJECTED_BY_NETWORK                = 135,
    QMI_VOICE_CALL_END_REASON_DEFLECTION_TO_SERVED_SUBSCRIBER    = 136,
    QMI_VOICE_CALL_END_REASON_SPECIAL_SERVICE_CODE               = 137,
    QMI_VOICE_CALL_END_REASON_INVALID_DEFLECTED_TO_NUMBER        = 138,
    QMI_VOICE_CALL_END_REASON_MULTIPARTY_PARTICIPANTS_EXCEEDED   = 139,
    QMI_VOICE_CALL_END_REASON_RESOURCES_NOT_AVAILABLE            = 140,
    QMI_VOICE_CALL_END_REASON_UNASSIGNED_NUMBER                  = 141,
    QMI_VOICE_CALL_END_REASON_NO_ROUTE_TO_DESTINATION            = 142,
    QMI_VOICE_CALL_END_REASON_CHANNEL_UNACCEPTABLE               = 143,
    QMI_VOICE_CALL_END_REASON_OPERATOR_DETERMINED_BARRING        = 144,
    QMI_VOICE_CALL_END_REASON_NORMAL_CALL_CLEARING               = 145,
    QMI_VOICE_CALL_END_REASON_USER_BUSY                          = 146,
    QMI_VOICE_CALL_END_REASON_NO_USER_RESPONDING                 = 147,
    QMI_VOICE_CALL_END_REASON_USER_ALERTING_NO_ANSWER            = 148,
    QMI_VOICE_CALL_END_REASON_CALL_REJECTED                      = 149,
    QMI_VOICE_CALL_END_REASON_NUMBER_CHANGED                     = 150,
    QMI_VOICE_CALL_END_REASON_PREEMPTION                         = 151,
    QMI_VOICE_CALL_END_REASON_DESTINATION_OUT_OF_ORDER           = 152,
    QMI_VOICE_CALL_END_REASON_INVALID_NUMBER_FORMAT              = 153,
    QMI_VOICE_CALL_END_REASON_FACILITY_REJECTED                  = 154,
    QMI_VOICE_CALL_END_REASON_RESPONSE_TO_STATUS_ENQUIRY         = 155,
    QMI_VOICE_CALL_END_REASON_NORMAL_UNSPECIFIED                 = 156,
    QMI_VOICE_CALL_END_REASON_NO_CIRCUIT_OR_CHANNEL_AVAILABLE    = 157,
    QMI_VOICE_CALL_END_REASON_NETWORK_OUT_OF_ORDER               = 158,
    QMI_VOICE_CALL_END_REASON_TEMPORARY_FAILURE                  = 159,
    QMI_VOICE_CALL_END_REASON_SWITCHING_EQUIPMENT_CONGESTION     = 160,
    QMI_VOICE_CALL_END_REASON_ACCESS_INFORMATION_DISCARDED       = 161,
    QMI_VOICE_CALL_END_REASON_REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE      = 162,
    QMI_VOICE_CALL_END_REASON_RESOURCES_UNAVAILABLE_OR_UNSPECIFIED            = 163,
    QMI_VOICE_CALL_END_REASON_QOS_UNAVAILABLE                                 = 164,
    QMI_VOICE_CALL_END_REASON_REQUESTED_FACILITY_NOT_SUBSCRIBED               = 165,
    QMI_VOICE_CALL_END_REASON_INCOMING_CALLS_BARRED_WITHIN_CUG                = 166,
    QMI_VOICE_CALL_END_REASON_BEARER_CAPABILITY_NOT_AUTH                      = 167,
    QMI_VOICE_CALL_END_REASON_BEARER_CAPABILITY_UNAVAILABLE                   = 168,
    QMI_VOICE_CALL_END_REASON_SERVICE_OPTION_NOT_AVAILABLE                    = 169,
    QMI_VOICE_CALL_END_REASON_ACM_LIMIT_EXCEEDED                              = 170,
    QMI_VOICE_CALL_END_REASON_BEARER_SERVICE_NOT_IMPLEMENTED                  = 171,
    QMI_VOICE_CALL_END_REASON_REQUESTED_FACILITY_NOT_IMPLEMENTED              = 172,
    QMI_VOICE_CALL_END_REASON_ONLY_DIGITAL_INFORMATION_BEARER_AVAILABLE       = 173,
    QMI_VOICE_CALL_END_REASON_SERVICE_OR_OPTION_NOT_IMPLEMENTED               = 174,
    QMI_VOICE_CALL_END_REASON_INVALID_TRANSACTION_IDENTIFIER                  = 175,
    QMI_VOICE_CALL_END_REASON_USER_NOT_MEMBER_OF_CUG                          = 176,
    QMI_VOICE_CALL_END_REASON_INCOMPATIBLE_DESTINATION                        = 177,
    QMI_VOICE_CALL_END_REASON_INVALID_TRANSIT_NETWORK_SELECTION               = 178,
    QMI_VOICE_CALL_END_REASON_SEMANTICALLY_INCORRECT_MESSAGE                  = 179,
    QMI_VOICE_CALL_END_REASON_INVALID_MANDATORY_INFORMATION                   = 180,
    QMI_VOICE_CALL_END_REASON_MESSAGE_TYPE_NOT_IMPLEMENTED                    = 181,
    QMI_VOICE_CALL_END_REASON_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE = 182,
    QMI_VOICE_CALL_END_REASON_INFORMATION_ELEMENT_NON_EXISTENT                = 183,
    QMI_VOICE_CALL_END_REASON_CONDITIONAL_IE_ERROR                            = 184,
    QMI_VOICE_CALL_END_REASON_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE      = 185,
    QMI_VOICE_CALL_END_REASON_RECOVERY_ON_TIMER_EXPIRED          = 186,
    QMI_VOICE_CALL_END_REASON_PROTOCOL_ERROR_UNSPECIFIED         = 187,
    QMI_VOICE_CALL_END_REASON_INTERWORKING_UNSPECIFIED           = 188,
    QMI_VOICE_CALL_END_REASON_OUTGOING_CALLS_BARRED_WITHIN_CUG   = 189,
    QMI_VOICE_CALL_END_REASON_NO_CUG_SELECTION                   = 190,
    QMI_VOICE_CALL_END_REASON_UNKNOWN_CUG_INDEX                  = 191,
    QMI_VOICE_CALL_END_REASON_CUG_INDEX_INCOMPATIBLE             = 192,
    QMI_VOICE_CALL_END_REASON_CUG_CALL_FAILURE_UNSPECIFIED       = 193,
    QMI_VOICE_CALL_END_REASON_CLIR_NOT_SUBSCRIBED                = 194,
    QMI_VOICE_CALL_END_REASON_CCBS_POSSIBLE                      = 195,
    QMI_VOICE_CALL_END_REASON_CCBS_NOT_POSSIBLE                  = 196,
    QMI_VOICE_CALL_END_REASON_IMSI_UNKNOWN_IN_HLR                = 197,
    QMI_VOICE_CALL_END_REASON_ILLEGAL_MS                         = 198,
    QMI_VOICE_CALL_END_REASON_IMSI_UNKNOWN_IN_VLR                = 199,
    QMI_VOICE_CALL_END_REASON_IMEI_NOT_ACCEPTED                  = 200,
    QMI_VOICE_CALL_END_REASON_ILLEGAL_ME                         = 201,
    QMI_VOICE_CALL_END_REASON_PLMN_NOT_ALLOWED                   = 202,
    QMI_VOICE_CALL_END_REASON_LOCATION_AREA_NOT_ALLOWED          = 203,
    QMI_VOICE_CALL_END_REASON_ROAMING_NOT_ALLOWED_IN_THIS_LOCATION_AREA = 204,
    QMI_VOICE_CALL_END_REASON_NO_SUITABLE_CELLS_IN_LOCATION_AREA = 205,
    QMI_VOICE_CALL_END_REASON_NETWORK_FAILURE                    = 206,
    QMI_VOICE_CALL_END_REASON_MAC_FAILURE                        = 207,
    QMI_VOICE_CALL_END_REASON_SYNCH_FAILURE                      = 208,
    QMI_VOICE_CALL_END_REASON_NETWORK_CONGESTION                 = 209,
    QMI_VOICE_CALL_END_REASON_GSM_AUTHENTICATION_UNACCEPTABLE    = 210,
    QMI_VOICE_CALL_END_REASON_SERVICE_NOT_SUBSCRIBED             = 211,
    QMI_VOICE_CALL_END_REASON_SERVICE_TEMPORARILY_OUT_OF_ORDER   = 212,
    QMI_VOICE_CALL_END_REASON_CALL_CANNOT_BE_IDENTIFIED          = 213,
    QMI_VOICE_CALL_END_REASON_INCORRECT_SEMANTICS_IN_MESSAGE     = 214,
    QMI_VOICE_CALL_END_REASON_MANDATORY_INFORMATION_INVALID      = 215,
    QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_FAILURE             = 216,
    QMI_VOICE_CALL_END_REASON_INVALID_SIM                        = 217,
    QMI_VOICE_CALL_END_REASON_WRONG_STATE                        = 218,
    QMI_VOICE_CALL_END_REASON_ACCESS_CLASS_BLOCKED               = 219,
    QMI_VOICE_CALL_END_REASON_NO_RESOURCES                       = 220,
    QMI_VOICE_CALL_END_REASON_INVALID_USER_DATA                  = 221,
    QMI_VOICE_CALL_END_REASON_TIMER_T3230_EXPIRED                = 222,
    QMI_VOICE_CALL_END_REASON_NO_CELL_AVAILABLE                  = 223,
    QMI_VOICE_CALL_END_REASON_ABORT_MESSAGE_RECEIVED             = 224,
    QMI_VOICE_CALL_END_REASON_RADIO_LINK_LOST                    = 225,
    QMI_VOICE_CALL_END_REASON_TIMER_T303_EXPIRED                 = 226,
    QMI_VOICE_CALL_END_REASON_CNM_MM_RELEASE_PENDING             = 227,
    QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_REJECT_RR_RELEASE_INDICATION                = 228,
    QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_REJECT_RR_RANDOM_ACCESS_FAILURE             = 229,
    QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_REJECT_RRC_RELEASE_INDICATION               = 230,
    QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_REJECT_RRC_CLOSE_SESSION_INDICATION         = 231,
    QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_REJECT_RRC_OPEN_SESSION_FAILURE             = 232,
    QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_REJECT_LOW_LEVEL_FAILURE                    = 233,
    QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_REJECT_LOW_LEVEL_FAILURE_REDIAL_NOT_ALLOWED = 234,
    QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_REJECT_LOW_LEVEL_IMMEDIATE_RETRY            = 235,
    QMI_VOICE_CALL_END_REASON_ACCESS_STRATUM_REJECT_ABORT_RADIO_UNAVAILABLE              = 236,
    QMI_VOICE_CALL_END_REASON_SERVICE_OPTION_NOT_SUPPORTED       = 237,
    QMI_VOICE_CALL_END_REASON_BAD_REQUEST_WAIT_INVITE            = 300,
    QMI_VOICE_CALL_END_REASON_BAD_REQUEST_WAIT_REINVITE          = 301,
    QMI_VOICE_CALL_END_REASON_INVALID_REMOTE_URI                 = 302,
    QMI_VOICE_CALL_END_REASON_REMOTE_UNSUPPORTED_MEDIA_TYPE      = 303,
    QMI_VOICE_CALL_END_REASON_PEER_NOT_REACHABLE                 = 304,
    QMI_VOICE_CALL_END_REASON_NETWORK_NO_RESPONSE_TIMEOUT        = 305,
    QMI_VOICE_CALL_END_REASON_NETWORK_NO_RESPONSE_HOLD_FAILURE   = 306,
    QMI_VOICE_CALL_END_REASON_DATA_CONNECTION_LOST               = 307,
    QMI_VOICE_CALL_END_REASON_UPGRADE_DOWNGRADE_REJECTED         = 308,
    QMI_VOICE_CALL_END_REASON_SIP_403_FORBIDDEN                  = 309,
    QMI_VOICE_CALL_END_REASON_NO_NETWORK_RESPONSE                = 310,
    QMI_VOICE_CALL_END_REASON_UPGRADE_DOWNGRADE_FAILED           = 311,
    QMI_VOICE_CALL_END_REASON_UPGRADE_DOWNGRADE_CANCELLED        = 312,
    QMI_VOICE_CALL_END_REASON_SSAC_REJECT                        = 313,
    QMI_VOICE_CALL_END_REASON_THERMAL_EMERGENCY                  = 314,
    QMI_VOICE_CALL_END_REASON_1XCSFB_SOFT_FAILURE                = 315,
    QMI_VOICE_CALL_END_REASON_1XCSFB_HARD_FAILURE                = 316,
} QmiVoiceCallEndReason;

/**
 * qmi_voice_call_end_reason_get_string:
 *
 * Since: 1.26
 */

/**
 * QmiVoiceCallControlResultType:
 * @QMI_VOICE_CALL_CONTROL_RESULT_TYPE_VOICE: Voice.
 * @QMI_VOICE_CALL_CONTROL_RESULT_TYPE_SUPS: Supplementary service.
 * @QMI_VOICE_CALL_CONTROL_RESULT_TYPE_USSD: Unstructured supplementary service.
 *
 * Call control result type.
 *
 * Since: 1.26
 */
typedef enum {
    QMI_VOICE_CALL_CONTROL_RESULT_TYPE_VOICE = 0x00,
    QMI_VOICE_CALL_CONTROL_RESULT_TYPE_SUPS  = 0x01,
    QMI_VOICE_CALL_CONTROL_RESULT_TYPE_USSD  = 0x02,
} QmiVoiceCallControlResultType;

/**
 * qmi_voice_call_control_result_type_get_string:
 *
 * Since: 1.26
 */

/**
 * QmiVoiceCallControlSupplementaryServiceType:
 * @QMI_VOICE_CALL_CONTROL_SUPPLEMENTARY_SERVICE_TYPE_ACTIVATE: Activate.
 * @QMI_VOICE_CALL_CONTROL_SUPPLEMENTARY_SERVICE_TYPE_DEACTIVATE: Deactivate.
 * @QMI_VOICE_CALL_CONTROL_SUPPLEMENTARY_SERVICE_TYPE_REGISTER: Register.
 * @QMI_VOICE_CALL_CONTROL_SUPPLEMENTARY_SERVICE_TYPE_ERASE: Erase.
 * @QMI_VOICE_CALL_CONTROL_SUPPLEMENTARY_SERVICE_TYPE_INTERROGATE: Interrogate.
 * @QMI_VOICE_CALL_CONTROL_SUPPLEMENTARY_SERVICE_TYPE_REGISTER_PASSWORD: Register password.
 * @QMI_VOICE_CALL_CONTROL_SUPPLEMENTARY_SERVICE_TYPE_USSD: USSD.
 *
 * Call control supplementary service type.
 *
 * Since: 1.26
 */
typedef enum {
    QMI_VOICE_CALL_CONTROL_SUPPLEMENTARY_SERVICE_TYPE_ACTIVATE          = 0x01,
    QMI_VOICE_CALL_CONTROL_SUPPLEMENTARY_SERVICE_TYPE_DEACTIVATE        = 0x02,
    QMI_VOICE_CALL_CONTROL_SUPPLEMENTARY_SERVICE_TYPE_REGISTER          = 0x03,
    QMI_VOICE_CALL_CONTROL_SUPPLEMENTARY_SERVICE_TYPE_ERASE             = 0x04,
    QMI_VOICE_CALL_CONTROL_SUPPLEMENTARY_SERVICE_TYPE_INTERROGATE       = 0x05,
    QMI_VOICE_CALL_CONTROL_SUPPLEMENTARY_SERVICE_TYPE_REGISTER_PASSWORD = 0x06,
    QMI_VOICE_CALL_CONTROL_SUPPLEMENTARY_SERVICE_TYPE_USSD              = 0x07,
} QmiVoiceCallControlSupplementaryServiceType;

/**
 * qmi_voice_call_control_supplementary_service_type_get_string:
 *
 * Since: 1.26
 */

#endif /* _LIBQMI_GLIB_QMI_ENUMS_VOICE_H_ */
