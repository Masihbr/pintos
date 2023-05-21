# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_EXIT_CODES => 1, [<<'EOF']);
(cache-hitrate) begin
(cache-hitrate) create "data2.bin"
(cache-hitrate) open "data2.bin"
(cache-hitrate) Going to empty cache and reset stats.
(cache-hitrate) Going to write data in file.
(cache-hitrate) About 200 block writes.
(cache-hitrate) No read should have been made.
(cache-hitrate) end
EOF
pass;
