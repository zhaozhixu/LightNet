# Hacking

## Commit Guidelines

* [Submit an issue](https://github.com/zhaozhixu/LightNet/issues/new) if you
found a bug or have a feature request.
* [Fork](https://github.com/zhaozhixu/LightNet/fork) and 
[open a pull request](https://github.com/zhaozhixu/LightNet/compare)
when you prepared to contribute. Before that, it is encouraged to open an 
issue to discuss.
* When committing to the git repository make sure to include a meaningful 
commit message. Commit messages should have the following format:

        Short explanation of the commit
    
        Longer explanation explaining exactly what's changed, whether any
        external or private interfaces changed, what bugs were fixed (with bug
        tracker reference if applicable) and so on. Be concise but not too brief.
    
## Code Conventions

* Indent the C code with 4 spaces.
* Use `/* */` comments in the C code.
* The limit on the length of lines is 80 columns and this is a strongly
  preferred limit, unless exceeding 80 columns significantly increases 
  readability. However, never break user-visible strings such as `printf`
  messages.
* Placing Braces
    1. Put the opening brace last on the line except namely functions. 
    2. Put the closing brace first except in the cases where it is followed by a
    continuation of the same statement, ie a while in a do-statement or an else
    in an if-statement.
    3. Do not unnecessarily use braces where a single statement will do except 
    that only one branch of a conditional statement is a single statement.
  
    E.g.:
    
        :::c
        int function(int x)
        {
            if (condition) {
                do a;
                do b;
            }

            if (condition)
                do e;

            if (condition) {
                do f;
                do g;
            } else {
                do h;
            }

            do {
                do c;
                do d;
            } while (condition);
        }

* Spaces
    1. Use a space after control-flow keywords:
    `if, switch, case, for, do, while`.
    2. Use a space after each comma.
    3. Use spaces around most binary and ternary operators but no space
    after unary operators.
    4. Avoid trailing space.
* Don't hide pointers with `typedef`. Bad example:
`typedef struct foo * foo;`
* As a general rule of thumb, follow the same coding style as the surrounding
code.
