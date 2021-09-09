/
* @file toml.q
* @overview Load a shared library to parse TOML file.
\

/
* @brief User defined path to tomlq.so. Must be absolute path. The default value is ${QHOME}/[os].
\
Q_LIBRARY_PATH: $[` ~ `$getenv `Q_LIBRARY_PATH; `tomlq; .Q.dd[hsym `$getenv `Q_LIBRARY_PATH; `tomlq]];

/
* @brief Parse a TOML file.
* @param file_path {symbol}: File handle to a TOML file.
* @return 
* - dictionary: Parsed document.
\
.toml.load: Q_LIBRARY_PATH 2: (`load_toml; 1);
