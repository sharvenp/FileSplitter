# FileSplitter

A simple C library to split a file into smaller files with a method to reassemble it.

This is useful when trying to send a large file concurrently, and even storing some files securely.

# Scripts

### split.c

`Usage: split <file_path> <number_of_chunks> <save_dir>`
- `<file_path>: The path to the file that needs to be split`
- `<number_of_chunks>: The number of chunks the file needs to be split into`
	- `Note that if it doesn't evenly divide the file size, an additional chunk will be created`
- `<save_dir>: The directory where all the chunks will be saved (format: <file_name>-<i>) `

This script will split the given file into `<number_of_chunks>` chunks. If it doesn't evenly divide the file size, an additional chunk will be created. In addition to this, a metadata file will be generated called `<file_name>-metadata`. This file contains information needed to re-assemble the file.

Do not delete or misplace the chunk files as re-assembly is impossible without it. 


### assemble.c
`Usage: assemble <meta_file> <save_file>`
- `<meta_file>: The path to the metadata file`
- `<save_file>: The file where the re-assembled file should be saved to`

This script will take in a metadata file and re-assemble it into the original file.

**Note that this may not work with all file extensions.**

# License

