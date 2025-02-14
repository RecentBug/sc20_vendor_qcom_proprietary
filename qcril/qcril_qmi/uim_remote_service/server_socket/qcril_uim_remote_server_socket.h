/*!
  @file
  qcril_uim_remote_server_socket.h

  @brief
  defines UIM remote Server socket APIs
    1. start the socket
    2. send messages(resp/ind) to the service on the app layer
    3. send unsol resp to the service on the app layer

*/

/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

#ifndef QCRIL_UIM_REMOTE_SERVER_SOCKET_H
#define QCRIL_UIM_REMOTE_SERVER_SOCKET_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "qcrili.h"

#ifndef QMI_RIL_UTF
  #include <hardware/ril/librilutils/proto/sap-api.pb.h>
#else
  #include "sap-api.pb.h"
#endif

#define QCRIL_UIM_REMOTE_SERVER_SOCKET_MAX_BUF_SIZE (1024*8)
#define QCRIL_UIM_REMOTE_SERVER_SOCKET_MAX_THREAD_NAME_SIZE 50

void qcril_uim_remote_server_socket_init(int argc, char **argv);

void qcril_uim_remote_server_socket_dispatch_request(const qcril_request_params_type *const params_ptr,
                                                    qcril_request_return_type *const ret_ptr);

#ifdef  __cplusplus
}
#endif

#endif /* QCRIL_UIM_REMOTE_CLIENT_SOCKET_H */
