#!/bin/bash
#
# Compare actual output with reference output specified by the RFCs.

diff <($TEST_SRCDIR/org_deepmind_lab/third_party/md/md4driver -x) $TEST_SRCDIR/org_deepmind_lab/third_party/md/golden.md4 || exit 1
diff <($TEST_SRCDIR/org_deepmind_lab/third_party/md/md5driver -x) $TEST_SRCDIR/org_deepmind_lab/third_party/md/golden.md5 || exit 1
