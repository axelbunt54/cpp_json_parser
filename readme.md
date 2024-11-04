## JSON Expression Parser

### Description

Developed basic JSON parser in C++ that supports trivial expressions with JSON paths.

[Original task](task.md). Done first point.

### Getting started with project

Clone repository:

```
git clone https://github.com/axelbunt/cpp_json_parser.git
```

Compile files:

```
g++ main.cpp jsonParser.cpp -o json_eval
```

Run built app:

```
# test,json: {"a": { "b": [ 1, 2, { "c": "test" }, [11, 12] ]}}
./json_eval "test/test.json" "a.b[1]"
# 2
```

### Developer zone

Run integration tests:

```
# First argument - path to test runner, 
# second argument - path to built app
python3 test/run_tests.py json_eval
```

### Known issues

I want to fix these issues later:

1. Redesign *JSONValue* `<optional>` fields to `union`.

2. Use link params in functions:
    - `json::lexify`,
    - `json::parse`,
    - `json::lexify_and_parse`,
    - `json::stringify`,
    - `json::get_json_value_by_path`.

3. Handle nested quotes in `json::lex_string`.

4. Add support for other data types for numbers in `json::parse`: 
    - `long`,
    - `float`,
    - `short`,
    - `byte`,
    - ...

5. Add JSON expression check in `json::lex_expression`.
