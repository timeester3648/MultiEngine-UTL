input_folder="include/UTL/*.hpp"
output_file="single_include/UTL.hpp"

for f in $input_folder; do
    [[ "$f" = $output_file ]] || { cat -- "$f"; printf "\n\n\n\n\n\n"; }
done > $output_file
