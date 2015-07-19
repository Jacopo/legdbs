#!/bin/bash

set -eux

diff <(./test_vanilla 2>/dev/null) test_vanilla.expected_out
diff <(./test 2>/dev/null) test.expected_out

echo 'Test passed :)'
