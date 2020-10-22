Dragonfly的使用C++编写。通过cmake进行构建。

源代码的根目录为`src`文件夹

编译器的入点为`main.cpp`。
除了此文件外其他文件都分布在`frontend`,`AST`,`LLVM`,`backend`文件夹。

##  frontend
frontend文件夹负责编译器前端。
其中包含
* lexer 词法分析器

    词法分析器会将输入字符串转化成单独的Token。
* parser 语法分析器

    语法分析器将构建出抽象语法树(AST)。
* debug 调试器

    调试器用于截取与输出编译时的错误信息。
* package-manager 包管理器

    包管理器用于载入第三方代码
* preprocessor 预处理器

    预处理器目前用作将多个文件合并成一个输入字符串。
* keyword 关键词
##  AST

## LLVM

## backend
当然没有任何内容。