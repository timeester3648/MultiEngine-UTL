input_folder="source/*.hpp"
output_file="include/proto_utils.hpp"

for f in $input_folder; do
    [[ "$f" = $output_file ]] || { cat -- "$f"; printf "\n\n\n\n\n\n"; }
done > $output_file
