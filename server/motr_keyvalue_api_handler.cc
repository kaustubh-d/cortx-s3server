/*
 * Copyright (c) 2020 Seagate Technology LLC and/or its Affiliates
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For any questions about this software or licensing,
 * please email opensource@seagate.com or cortx-questions@seagate.com.
 *
 */

#include "motr_api_handler.h"
#include "motr_get_key_value_action.h"
#include "motr_put_key_value_action.h"
#include "motr_delete_key_value_action.h"
#include "s3_log.h"
#include "s3_stats.h"

void MotrKeyValueAPIHandler::create_action() {
  s3_log(S3_LOG_DEBUG, request_id, "Entering\n");
  s3_log(S3_LOG_DEBUG, request_id, "Operation code = %d\n", operation_code);

  switch (operation_code) {
    case MotrOperationCode::none:
      // Perform operation on Object.
      switch (request->http_verb()) {
        case S3HttpVerb::GET:
          action = std::make_shared<MotrGetKeyValueAction>(request);
          s3_stats_inc("motr_http_get_keyvalue_request_count");
          break;
        case S3HttpVerb::PUT:
          action = std::make_shared<MotrPutKeyValueAction>(request);
          s3_stats_inc("motr_http_put_keyvalue_request_count");
          break;
        case S3HttpVerb::DELETE:
          action = std::make_shared<MotrDeleteKeyValueAction>(request);
          s3_stats_inc("motr_http_delete_keyvalue_request_count");
          break;
        default:
          // should never be here.
          return;
      };
      break;
    default:
      // should never be here.
      return;
  };  // switch operation_code
  s3_log(S3_LOG_DEBUG, "", "Exiting\n");
}
