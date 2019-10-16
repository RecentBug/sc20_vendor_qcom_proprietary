#ifndef _SNS_FILE_H_
#define _SNS_FILE_H_

/*============================================================================
  @file sns_file.h

  @brief Header file for function declarations and LA-specific defines
  of the file service.

  <br><br>

  Copyright (c) 2013, 2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ============================================================================*/

/*============================================================================
  INCLUDE FILES
=============================================================================*/

#include "sns_file_internal_v01.h"
#include "sns_common.h"
#include "sns_smr_util.h"

/*===========================================================================
                  DEFINES
============================================================================*/

#ifdef SNS_LA_SIM
#define DEBUG_FILE_DIR_BASE "/tmp/"
#else /* SNS_LA_SIM */
#define DEBUG_FILE_DIR_BASE "/persist/sensors/"
#endif /* !SNS_LA_SIM */
#define DEBUG_FILE_DIR_BASE_LEN 25
#define ERR_LOG_FILE_NAME "error_log"

/*============================================================================

  FUNCTION:   sns_file_csi_connect

  ==========================================================================*/
/*!
  @brief Handler for client connection.

  @param connection_handle[io] client's handle.

  @return Error code

*/
/*==========================================================================*/
sns_err_code_e sns_file_csi_connect( void **connection_handle);


/*============================================================================

  FUNCTION:   sns_file_csi_disconnect

  ==========================================================================*/
/*!
  @brief Handler for client disconnection.

  @param connection_handle[i] client handle.

  @return Error code

*/
/*==========================================================================*/
sns_err_code_e sns_file_csi_disconnect( void *connection_handle);

/*============================================================================

  FUNCTION:   sns_file_handle

  ==========================================================================*/
/*!
  @brief Handler for incoming file service request messages.

  @param smr_header[io] Header associated with the request message.
  @param connection_handle[i] client's handle
  @param req_msg_ptr[i] Request message
  @param resp_msg_ptr[o] Response message generated by the file service.

  @return Error code

  @note The memory for req_msg_ptr may be used to hold the response if
        memory for the response cannot be allocated.
*/
/*==========================================================================*/
sns_err_code_e sns_file_handle( sns_smr_header_s *smr_header,
                                void *connection_handle,
                                void *msg_ptr,
                                void **resp_msg_ptr );

/*============================================================================
  FUNCTION:   sns_file_mr_init
  ==========================================================================*/
/*!
  @brief Initialize message router data structures and connections.

  @return
  SNS_SUCCESS or error code.
*/
/*==========================================================================*/
sns_err_code_e sns_file_mr_init( void );

/*============================================================================
  FUNCTION:   sns_file_mr_deinit
  ==========================================================================*/
/*!
  @brief Deinitialize message router data structures and connections.

  @return
  SNS_SUCCESS or error code.
*/
/*==========================================================================*/
sns_err_code_e sns_file_mr_deinit( void );

/*============================================================================
  FUNCTION:   sns_file_mr_thread
  ==========================================================================*/
/*!
  @brief Main thread loop for the Sensors File task.

  @param[i] p_arg: Argument passed in during thread creation. Unused.

  @return
  None.
*/
/*==========================================================================*/
void sns_file_mr_thread( void *p_arg );

#endif /* _SNS_FILE_H_ */
