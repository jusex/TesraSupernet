/*
 * simple_buffer.c
 * Copyright  : Kyle Harper
 * License    : Follows same licensing as the lz4.c/lz4.h program at any given time.  Currently, BSD 2.
 * Description: Example program to demonstrate the basic usage of the compress/decompress functions within lz4.c/lz4.h.
 *              The functions you'll likely want are LZ4_compress_default and LZ4_decompress_safe.  Both of these are documented in
 *              the lz4.h header file; I recommend reading them.
 */


#include "lz4.h"     
#include <stdio.h>   
#include <string.h>  
#include <stdlib.h>  

/*
 * Easy show-error-and-bail function.
 */
void run_screaming(const char *message, const int code) {
  printf("%s\n", message);
  exit(code);
  return;
}


/*
 * main
 */
int main(void) {
  
  
  
  
  
  
  
  
  

  
  
  const char* const src = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.";
  
  const int src_size = (int)(strlen(src) + 1);
  
  const int max_dst_size = LZ4_compressBound(src_size);
  
  char* compressed_data = malloc(max_dst_size);
  if (compressed_data == NULL)
    run_screaming("Failed to allocate memory for *compressed_data.", 1);
  
  
  
  const int compressed_data_size = LZ4_compress_default(src, compressed_data, src_size, max_dst_size);
  
  if (compressed_data_size < 0)
    run_screaming("A negative result from LZ4_compress_default indicates a failure trying to compress the data.  See exit code (echo $?) for value returned.", compressed_data_size);
  if (compressed_data_size == 0)
    run_screaming("A result of 0 means compression worked, but was stopped because the destination buffer couldn't hold all the information.", 1);
  if (compressed_data_size > 0)
    printf("We successfully compressed some data!\n");
  
  
  compressed_data = (char *)realloc(compressed_data, compressed_data_size);
  if (compressed_data == NULL)
    run_screaming("Failed to re-alloc memory for compressed_data.  Sad :(", 1);

  
  
  
  char* const regen_buffer = malloc(src_size);
  if (regen_buffer == NULL)
    run_screaming("Failed to allocate memory for *regen_buffer.", 1);
  
  
  
  const int decompressed_size = LZ4_decompress_safe(compressed_data, regen_buffer, compressed_data_size, src_size);
  free(compressed_data);   
  if (decompressed_size < 0)
    run_screaming("A negative result from LZ4_decompress_safe indicates a failure trying to decompress the data.  See exit code (echo $?) for value returned.", decompressed_size);
  if (decompressed_size == 0)
    run_screaming("I'm not sure this function can ever return 0.  Documentation in lz4.h doesn't indicate so.", 1);
  if (decompressed_size > 0)
    printf("We successfully decompressed some data!\n");
  
  

  
  
  if (memcmp(src, regen_buffer, src_size) != 0)
    run_screaming("Validation failed.  *src and *new_src are not identical.", 1);
  printf("Validation done.  The string we ended up with is:\n%s\n", regen_buffer);
  return 0;
}
