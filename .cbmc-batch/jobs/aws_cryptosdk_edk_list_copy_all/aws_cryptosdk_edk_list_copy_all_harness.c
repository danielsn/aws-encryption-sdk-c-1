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

#include <aws/cryptosdk/edk.h>
#include <proof_helpers/cryptosdk/make_common_data_structures.h>
#include <proof_helpers/make_common_data_structures.h>
#include <proof_helpers/proof_allocators.h>
#include <proof_helpers/utils.h>

bool aws_array_list_is_valid(const struct aws_array_list *AWS_RESTRICT list) {
    if (!list) {
        return false;
    }

    bool data_is_valid =
        ((list->current_size == 0 && list->data == NULL) || AWS_MEM_IS_WRITABLE(list->data, list->current_size));
    bool item_size_is_valid = (list->item_size != 0);
    return data_is_valid && item_size_is_valid;
}

bool aws_array_list_is_valid_deep(const struct aws_array_list *AWS_RESTRICT list) {
    if (!list) {
        return false;
    }
    size_t required_size        = list->length * list->item_size;
    bool required_size_is_valid = true;
    //        (aws_mul_size_checked(list->length, list->item_size, &required_size) == AWS_OP_SUCCESS);
    bool current_size_is_valid = (list->current_size >= required_size);
    bool data_is_valid =
        ((list->current_size == 0 && list->data == NULL) || AWS_MEM_IS_WRITABLE(list->data, list->current_size));
    bool item_size_is_valid = (list->item_size != 0);
    return required_size_is_valid && current_size_is_valid && data_is_valid && item_size_is_valid;
}

// allocator, dest, src
typedef int (*clone_item_fn)(struct aws_allocator *, void *, const void *);
typedef void (*clean_up_item_fn)(void *);

int aws_cryptosdk_edk_init_clone(struct aws_allocator *alloc, void *dest, const void *src) {
    assert(AWS_MEM_IS_READABLE(src, sizeof(struct aws_cryptosdk_edk)));
    assert(AWS_MEM_IS_WRITABLE(dest, sizeof(struct aws_cryptosdk_edk)));
    uint8_t *d = (uint8_t *)dest;
    *d         = 0xab;
    return nondet_int();
}

void aws_cryptosdk_edk_clean_up(void *p) {
    assert(AWS_MEM_IS_READABLE(p, sizeof(struct aws_cryptosdk_edk)));
    uint8_t *d = (uint8_t *)p;
    assert(*d == 0xab);
}

void aws_cryptosdk_edk_list_copy_all_harness() {
    struct aws_array_list *dest = can_fail_malloc(sizeof(*dest));
    __CPROVER_assume(dest != NULL);
    __CPROVER_assume(dest->length <= NUM_ELEMS);
    __CPROVER_assume(dest->item_size == sizeof(struct aws_cryptosdk_edk));
    ensure_array_list_has_allocated_data_member(dest);
    __CPROVER_assume(aws_array_list_is_valid_deep(dest));

    struct aws_array_list *src = can_fail_malloc(sizeof(*src));
    __CPROVER_assume(src != NULL);
    __CPROVER_assume(src->length <= NUM_ELEMS);
    __CPROVER_assume(src->item_size == sizeof(struct aws_cryptosdk_edk));
    ensure_array_list_has_allocated_data_member(src);
    __CPROVER_assume(aws_array_list_is_valid_deep(src));

    const struct aws_array_list old_dest = *dest;
    const struct aws_array_list old_src  = *src;

    if (aws_cryptosdk_edk_list_copy_all(can_fail_allocator(), dest, src) == AWS_OP_SUCCESS) {
        assert(src->length == 0);
        assert(dest->length == old_dest.length + old_src.length);
    }
    assert(aws_array_list_is_valid(src));
    assert(aws_array_list_is_valid(dest));
}

#include <aws/common/error.inl>
