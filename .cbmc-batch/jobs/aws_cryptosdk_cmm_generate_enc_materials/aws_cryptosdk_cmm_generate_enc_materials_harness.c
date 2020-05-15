/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You may not
 * use
 * this file except in compliance with the License. A copy of the License is
 * located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed on
 * an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or
 * implied. See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <aws/cryptosdk/materials.h>
#include <aws/cryptosdk/private/keyring_trace.h>
#include <cbmc_invariants.h>
#include <cipher_openssl.h>
#include <make_common_data_structures.h>
#include <proof_helpers/cryptosdk/make_common_data_structures.h>
#include <proof_helpers/make_common_data_structures.h>
#include <proof_helpers/proof_allocators.h>
#include <proof_helpers/utils.h>

// Stub this until https://github.com/diffblue/cbmc/issues/5344 is fixed
// Original function is here:
// https://github.com/aws/aws-encryption-sdk-c/blob/master/source/edk.c#L44
void aws_cryptosdk_edk_list_clean_up(struct aws_array_list *encrypted_data_keys) {
    assert(aws_cryptosdk_edk_list_is_valid(encrypted_data_keys));
    aws_array_list_clean_up(encrypted_data_keys);
}


/**
 * Receives encryption request from user and attempts to generate encryption materials,
 * including an encrypted data key and a list of EDKs for doing encryption.
 *
 * On success returns AWS_OP_SUCCESS and allocates encryption materials object at address
 * pointed to by output.
 *
 * On failure returns AWS_OP_ERR, sets address pointed to by output to NULL, and sets
 * internal AWS error code.
 */
int generate_enc_materials(struct aws_cryptosdk_cmm *cmm,
			   struct aws_cryptosdk_enc_materials **output,
			   struct aws_cryptosdk_enc_request *request)
{
    //assert(aws_cryptosdk_cmm_base_is_valid(cmm));
    // assert(__CPROVER_w_ok(output, sizeof(*output)));
    //assert(aws_cryptosdk_enc_request_is_valid(request));
    
    struct aws_cryptosdk_enc_materials *materials = can_fail_malloc(sizeof(*materials));
    if(materials == NULL) {
	*output = NULL;
	return AWS_OP_ERR;
    }

    //Set up the allocator
    materials->alloc = request->alloc;
    __CPROVER_assume(aws_allocator_is_valid(materials->alloc));

    // Set up the signctx
    materials->signctx = can_fail_malloc(sizeof(*materials->signctx));
    if (materials->signctx) {
    	ensure_sig_ctx_has_allocated_members(materials->signctx);
    	__CPROVER_assume(aws_cryptosdk_sig_ctx_is_valid_cbmc(materials->signctx));
    }

    // Set up the unencrypted_data_key
    __CPROVER_assume(aws_byte_buf_is_bounded(&materials->unencrypted_data_key, MAX_NUM_ITEMS));
    ensure_byte_buf_has_allocated_buffer_member(&materials->unencrypted_data_key);

    // Set up the edk_list
    // edk_list Precondition: We have a valid list */
    __CPROVER_assume(aws_cryptosdk_edk_list_is_bounded(&materials->encrypted_data_keys, MAX_NUM_ITEMS));
    ensure_cryptosdk_edk_list_has_allocated_list(&materials->encrypted_data_keys);
    __CPROVER_assume(aws_cryptosdk_edk_list_is_valid(&materials->encrypted_data_keys));

    /* // Stub until https://github.com/diffblue/cbmc/issues/5344 is fixed */
    /* // edk_list Precondition: The list has valid list elements  */
    /* /\* */
    /*   __CPROVER_assume(aws_cryptosdk_edk_list_elements_are_bounded(&materials->encrypted_data_keys, MAX_STRING_LEN)); */
    /*   ensure_cryptosdk_edk_list_has_allocated_list_elements(&materials->encrypted_data_keys); */
    /*   __CPROVER_assume(aws_cryptosdk_edk_list_elements_are_valid(&materials->encrypted_data_keys)); */
    /* *\/ */
    /* // Set up the keyring trace */
    /* __CPROVER_assume(aws_array_list_is_bounded( */
    /* 					       &materials->keyring_trace, MAX_NUM_ITEMS, sizeof(struct aws_cryptosdk_keyring_trace_record))); */
    /* __CPROVER_assume(materials->keyring_trace.item_size == sizeof(struct aws_cryptosdk_keyring_trace_record)); */
    /* ensure_array_list_has_allocated_data_member(&materials->keyring_trace); */
    /* __CPROVER_assume(aws_array_list_is_valid(&materials->keyring_trace)); */
    /* ensure_trace_has_allocated_records(&materials->keyring_trace, MAX_STRING_LEN); */
    /* __CPROVER_assume(aws_cryptosdk_keyring_trace_is_valid(&materials->keyring_trace)); */

    *output = materials;
    return AWS_OP_SUCCESS;
}  

void aws_cryptosdk_cmm_generate_enc_materials_harness() {

    
    const struct aws_cryptosdk_cmm_vt vtable = { .vt_size                = sizeof(struct aws_cryptosdk_cmm_vt),
						 .name                   = ensure_c_str_is_allocated(SIZE_MAX),
						 .destroy                = nondet_voidp(),
						 .generate_enc_materials = nondet_bool() ? generate_enc_materials : NULL,
						 .decrypt_materials      = nondet_voidp() };
    __CPROVER_assume(aws_cryptosdk_cmm_vtable_is_valid(&vtable));

    struct aws_cryptosdk_cmm *cmm = can_fail_malloc(sizeof(struct aws_cryptosdk_cmm));

    if (cmm) {
	cmm->vtable = &vtable;
	__CPROVER_assume(aws_cryptosdk_cmm_base_is_valid(cmm));
    }

    //TODO
    __CPROVER_assume(cmm);
    
    struct aws_cryptosdk_enc_request request;
    request.alloc = can_fail_allocator();

    struct aws_cryptosdk_enc_materials **output = can_fail_malloc(sizeof(*output));

    // Run the function under test.
    aws_cryptosdk_cmm_generate_enc_materials(cmm, output, &request);
}
