/*
 * COPYRIGHT 2015 SEAGATE LLC
 *
 * THIS DRAWING/DOCUMENT, ITS SPECIFICATIONS, AND THE DATA CONTAINED
 * HEREIN, ARE THE EXCLUSIVE PROPERTY OF SEAGATE TECHNOLOGY
 * LIMITED, ISSUED IN STRICT CONFIDENCE AND SHALL NOT, WITHOUT
 * THE PRIOR WRITTEN PERMISSION OF SEAGATE TECHNOLOGY LIMITED,
 * BE REPRODUCED, COPIED, OR DISCLOSED TO A THIRD PARTY, OR
 * USED FOR ANY PURPOSE WHATSOEVER, OR STORED IN A RETRIEVAL SYSTEM
 * EXCEPT AS ALLOWED BY THE TERMS OF SEAGATE LICENSES AND AGREEMENTS.
 *
 * YOU SHOULD HAVE RECEIVED A COPY OF SEAGATE'S LICENSE ALONG WITH
 * THIS RELEASE. IF NOT PLEASE CONTACT A SEAGATE REPRESENTATIVE
 * http://www.seagate.com/contact
 *
 * Original author:  Kaustubh Deorukhkar   <kaustubh.deorukhkar@seagate.com>
 * Original author:  Rajesh Nambiar   <rajesh.nambiar@seagate.com>
 * Original creation date: 1-Oct-2015
 */

#include "s3_common.h"
#include<unistd.h>

#include "s3_clovis_rw_common.h"
#include "s3_clovis_config.h"
#include "s3_clovis_writer.h"
#include "s3_uri_to_mero_oid.h"
#include "s3_md5_hash.h"

extern struct m0_clovis_realm     clovis_uber_realm;

S3ClovisWriter::S3ClovisWriter(std::shared_ptr<S3RequestObject> req) : request(req), state(S3ClovisWriterOpState::start) {

}

void S3ClovisWriter::create_object(std::function<void(void)> on_success, std::function<void(void)> on_failed) {
printf("S3ClovisWriter::create_object\n");
  this->handler_on_success = on_success;
  this->handler_on_failed  = on_failed;

  writer_context.reset(new S3ClovisWriterContext(request, std::bind( &S3ClovisWriter::create_object_successful, this), std::bind( &S3ClovisWriter::create_object_failed, this)));

  struct s3_clovis_op_context *ctx = writer_context->get_clovis_op_ctx();

  ctx->cbs->ocb_arg = (void *)writer_context.get();
  ctx->cbs->ocb_executed = NULL;
  ctx->cbs->ocb_stable = s3_clovis_op_stable;
  ctx->cbs->ocb_failed = s3_clovis_op_failed;

  S3UriToMeroOID(request->get_object_uri().c_str(), &id);

  m0_clovis_obj_init(ctx->obj, &clovis_uber_realm, &id);

  m0_clovis_entity_create(&(ctx->obj->ob_entity), &(ctx->ops[0]));

  m0_clovis_op_setup(ctx->ops[0], ctx->cbs, 0);
  m0_clovis_op_launch(ctx->ops, 1);

}

void S3ClovisWriter::create_object_successful() {
  printf("S3ClovisWriter::create_object_successful\n");
  state = S3ClovisWriterOpState::created;
  this->handler_on_success();
}

void S3ClovisWriter::create_object_failed() {
  printf("S3ClovisWriter::create_object_failed\n");
  if (writer_context->get_errno() == -EEXIST) {
    state = S3ClovisWriterOpState::exists;
  } else {
    state = S3ClovisWriterOpState::failed;
  }
  this->handler_on_failed();
}

void S3ClovisWriter::write_content(std::function<void(void)> on_success, std::function<void(void)> on_failed) {

  this->handler_on_success = on_success;
  this->handler_on_failed  = on_failed;

  size_t clovis_block_size = S3ClovisConfig::get_instance()->get_clovis_block_size();
  size_t clovis_block_count = (request->get_content_length() + (clovis_block_size - 1)) / clovis_block_size;

  writer_context.reset(new S3ClovisWriterContext(request, std::bind( &S3ClovisWriter::write_content_successful, this), std::bind( &S3ClovisWriter::write_content_failed, this)));

  writer_context->init_write_op_ctx(clovis_block_count, clovis_block_size);

  struct s3_clovis_op_context *ctx = writer_context->get_clovis_op_ctx();
  struct s3_clovis_rw_op_context *rw_ctx = writer_context->get_clovis_rw_op_ctx();

  ctx->cbs->ocb_arg = (void *)writer_context.get();
  ctx->cbs->ocb_executed = NULL;
  ctx->cbs->ocb_stable = s3_clovis_op_stable;
  ctx->cbs->ocb_failed = s3_clovis_op_failed;

  S3UriToMeroOID(request->get_object_uri().c_str(), &id);

  // Remove this. Ideally we should do
  // for i = 0 to block_count
  //   S3RequestObject::consume(ctx.data->ov_buf[i], block_size)
  set_up_clovis_data_buffers(rw_ctx);

  m0_clovis_obj_init(ctx->obj, &clovis_uber_realm, &id);

  /* Create the write request */
  m0_clovis_obj_op(ctx->obj, M0_CLOVIS_OC_WRITE,
       rw_ctx->ext, rw_ctx->data, rw_ctx->attr, 0, &ctx->ops[0]);

  m0_clovis_op_setup(ctx->ops[0], ctx->cbs, 0);
  m0_clovis_op_launch(ctx->ops, 1);
}

void S3ClovisWriter::write_content_successful() {
  state = S3ClovisWriterOpState::saved;
  this->handler_on_success();
}

void S3ClovisWriter::write_content_failed() {
  state = S3ClovisWriterOpState::failed;
  this->handler_on_failed();
}

void S3ClovisWriter::delete_object(std::function<void(void)> on_success, std::function<void(void)> on_failed) {
printf("S3ClovisWriter::delete_object\n");
  this->handler_on_success = on_success;
  this->handler_on_failed  = on_failed;

  writer_context.reset(new S3ClovisWriterContext(request, std::bind( &S3ClovisWriter::delete_object_successful, this), std::bind( &S3ClovisWriter::delete_object_failed, this)));

  struct s3_clovis_op_context *ctx = writer_context->get_clovis_op_ctx();

  ctx->cbs->ocb_arg = (void *)writer_context.get();
  ctx->cbs->ocb_executed = NULL;
  ctx->cbs->ocb_stable = s3_clovis_op_stable;
  ctx->cbs->ocb_failed = s3_clovis_op_failed;

  S3UriToMeroOID(request->get_object_uri().c_str(), &id);

  m0_clovis_obj_init(ctx->obj, &clovis_uber_realm, &id);

  m0_clovis_entity_delete(&(ctx->obj->ob_entity), &(ctx->ops[0]));

  m0_clovis_op_setup(ctx->ops[0], ctx->cbs, 0);
  m0_clovis_op_launch(ctx->ops, 1);

}

void S3ClovisWriter::delete_object_successful() {
  printf("S3ClovisWriter::delete_object_successful\n");
  state = S3ClovisWriterOpState::deleted;
  this->handler_on_success();
}

void S3ClovisWriter::delete_object_failed() {
  printf("S3ClovisWriter::delete_object_failed\n");
  state = S3ClovisWriterOpState::failed;
  this->handler_on_failed();
}

void S3ClovisWriter::set_up_clovis_data_buffers(struct s3_clovis_rw_op_context* rw_ctx) {
  // Copy the data to clovis buffers.
  // xxx - move to S3RequestObject::consume(char* ptr, 4k);
  size_t clovis_block_size = S3ClovisConfig::get_instance()->get_clovis_block_size();
  size_t clovis_block_count = (request->get_content_length() + (clovis_block_size - 1)) / clovis_block_size;

  int data_to_consume = clovis_block_size;
  int pending_from_current_extent = 0;
  int idx_within_block = 0;
  int idx_within_extent = 0;
  int current_block_idx = 0;
  uint64_t last_index = 0;
  MD5hash  md5crypt;
  char *destination = NULL;
  char *source = NULL;

  size_t num_of_extents = evbuffer_peek(request->buffer_in(), request->get_content_length() /*-1*/, NULL, NULL, 0);

  /* do the actual peek */
  struct evbuffer_iovec *vec_in = NULL;
  vec_in = (struct evbuffer_iovec *)malloc(num_of_extents * sizeof(struct evbuffer_iovec));

  /* do the actual peek at data */
  evbuffer_peek(request->buffer_in(), request->get_content_length(), NULL/*start of buffer*/, vec_in, num_of_extents);

  for (size_t i = 0; i < num_of_extents; i++) {
    // printf("processing extent = %d of total %d\n", i, data_extents);
    pending_from_current_extent = vec_in[i].iov_len;
    /* KD xxx - we need to avoid this copy */
    while (1) /* consume all data in extent */
    {
      if (pending_from_current_extent == data_to_consume)
      {
        /* consume all */
        // printf("Writing @ %d bytes from extent = [%d] in block [%d] at index [%d]\n", pending_from_current_extent, i, current_block_idx, idx_within_block);
        // printf("memcpy(%p, %p, %d)\n", data.ov_buf[current_block_idx] + idx_within_block, vec_in[i].iov_base + idx_within_extent, pending_from_current_extent);
        source = (char*)vec_in[i].iov_base + idx_within_extent;
        destination = (char*) rw_ctx->data->ov_buf[current_block_idx] + idx_within_block;

        memcpy(destination, source, pending_from_current_extent);
        md5crypt.Update((const char *)source, pending_from_current_extent);

        idx_within_block = idx_within_extent = 0;
        data_to_consume = clovis_block_size;
        current_block_idx++;
        break; /* while(1) */
      } else if (pending_from_current_extent < data_to_consume)
      {
        // printf("Writing @ %d bytes from extent = [%d] in block [%d] at index [%d]\n", pending_from_current_extent, i, current_block_idx, idx_within_block);
        // printf("memcpy(%p, %p, %d)\n", data.ov_buf[current_block_idx] + idx_within_block, vec_in[i].iov_base + idx_within_extent, pending_from_current_extent);
        source = (char*)vec_in[i].iov_base + idx_within_extent;
        destination = (char*)rw_ctx->data->ov_buf[current_block_idx] + idx_within_block;

        memcpy(destination, source, pending_from_current_extent);
        md5crypt.Update((const char *)source, pending_from_current_extent);

        idx_within_block = idx_within_block + pending_from_current_extent;
        idx_within_extent = 0;
        data_to_consume = data_to_consume - pending_from_current_extent;
        break; /* while(1) */
      } else /* if (pending_from_current_extent > data_to_consume) */
      {
        // printf("Writing @ %d bytes from extent = [%d] in block [%d] at index [%d]\n", data_to_consume, i, current_block_idx, idx_within_block);
        // printf("memcpy(%p, %p, %d)\n", data.ov_buf[current_block_idx] + idx_within_block, vec_in[i].iov_base + idx_within_extent, data_to_consume);
        source = (char*)vec_in[i].iov_base + idx_within_extent;
        destination = (char*)rw_ctx->data->ov_buf[current_block_idx] + idx_within_block;

        memcpy(destination, source, data_to_consume);
        md5crypt.Update((const char *)source, data_to_consume);

        idx_within_block = 0;
        idx_within_extent = idx_within_extent + data_to_consume;
        pending_from_current_extent = pending_from_current_extent - data_to_consume;
        data_to_consume = clovis_block_size;
        current_block_idx++;
      }
    }
  }

  // Complete MD5 computation and remember
  md5crypt.Finalize();
  this->content_md5 = md5crypt.get_md5_string();

  free(vec_in);

  // Init clovis buffer attrs.
  for(size_t i = 0; i < clovis_block_count; i++)
  {
    rw_ctx->ext->iv_index[i] = last_index;
    rw_ctx->ext->iv_vec.v_count[i] = /*vec_in[i].iov_len*/clovis_block_size;
    last_index += /*vec_in[i].iov_len*/clovis_block_size;

    /* we don't want any attributes */
    rw_ctx->attr->ov_vec.v_count[i] = 0;
  }

}
