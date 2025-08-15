#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <assert.h>
#include "instruction.h"


void usage() {
    fprintf(stderr, "Usage: ./xas file");
    exit(1);
}


int main(int argc, char** argv) {
    if (argc != 2) {
        usage();
    }

    // TODO: Your code here
}
