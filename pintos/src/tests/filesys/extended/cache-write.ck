# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_EXIT_CODES => 1, [<<'EOF']);
(cache-write) begin
(cache-write) Going create file.
(cache-write) create "data2.bin"
(cache-write) open "data2.bin"
(cache-write) Going to empty cache and reset stats.
(cache-write) Going to write data in file.
(cache-write) 200 block writes.
(cache-write) No read should have been made.
(cache-write) end
EOF
pass;
