#!/bin/bash

set -eux

diff <(./test_vanilla 2>&1) test_vanilla.expected_out
diff <(./test 2>&1) test.expected_out

echo 'Test passed :)'
