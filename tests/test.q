/
* @file test.q
* @overview Test tomlq library. 
* @note Run from `toml/` directory as below:
* ```
* tomlq]$ q tests/test.q 
* ```
\

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                    Initial Setting                    //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

// Set `Q_LIBRARY_PATH` to `tomlq/install/`.
setenv[`Q_LIBRARY_PATH; getenv[`PWD], "/install"];

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                     Load Libraries                    //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

// Load tomlq library and helper functions
\l q/toml.q
\l tests/test_helper_function.q

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                         Tests                         //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

// Parse test file
document: .toml.load `:file/test.toml;

.test.ASSERT_EQ["symbol"; document `file; `test.toml];
.test.ASSERT_EQ["string"; document `description; "This file is provided to test tomlq for possible TOML types. Adding can be no harm but\n subtracting is not acceptable.\n"];
.test.ASSERT_EQ["dictionary"; document `features; `build`code`test!(`CMake; `$"light-weight"; 1b)];
.test.ASSERT_EQ["bool"; document[`property; `$"read-only"]; 1b];
.test.ASSERT_EQ["symbol list"; document[`property; `tag]; `TOML`interface`q];
.test.ASSERT_EQ["date"; document[`record; `date]; 2021.09.09];
.test.ASSERT_EQ["timestamp"; document[`record; `timestamp]; 2021.09.09D14:29:20.525000000];
.test.ASSERT_EQ["time"; document[`record; `time]; 23:29:20.525];
.test.ASSERT_EQ["float"; document[`record; `$"temperature-float"]; 22.3];
.test.ASSERT_EQ["long"; document[`record; `$"temperature-int"]; 22];
.test.ASSERT_EQ["bool list"; document[`items; `bools]; 10b];
.test.ASSERT_EQ["int list"; document[`items; `ints]; 4000 2000 1000];
.test.ASSERT_EQ["float list"; document[`items; `doubles]; 1 2 0.5];
.test.ASSERT_EQ["timestamp list"; document[`items; `timestamps]; 1978.05.31D19:30:00.000000000 1998.12.31D05:00:00.888000000];
.test.ASSERT_EQ["date list"; document[`items; `dates]; 2009.02.18 2012.12.31 2015.03.16];
.test.ASSERT_EQ["time list"; document[`items; `times]; 04:15:00.000 13:00:00.000 19:30:30.000];
.test.ASSERT_EQ["list of lists"; document[`items; `arrays]; (7 14 21; 40 400)];
.test.ASSERT_EQ["list of dictionaries"; document[`items; `tables]; (`library`language!(`tomlq; `q`C); `OS`architecture`kernel!(`LINUX; `x86_64; 3.1))];
.test.ASSERT_EQ["compound list"; document[`items; `miscellaneous]; (1b; 7777; 3.5; `water; 2021.08.17D19:00:00.000000000; ())];

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                         Result                        //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

.test.DISPLAY_RESULT[];
exit 0;