# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_EXIT_CODES => 1, [<<'EOF']);
(cache-hitrate) begin
(cache-hitrate) create "data.bin"
(cache-hitrate) open "data.bin"
(cache-hitrate) Going to write data in file
(cache-hitrate) Going to empty cache and reset stats
(cache-hitrate) Going to read file 1st time
(cache-hitrate) Going to read file 2nd time
(cache-hitrate) Better hitrate after filling cache.
(cache-hitrate) No write should have been made.
(cache-hitrate) end
EOF
pass;
