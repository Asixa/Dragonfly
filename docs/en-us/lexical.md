# **Lexical Structure**

### Whitespace and Comments
Whitespace has two uses: to separate tokens in the source file and to help determine whether an operator is a prefix or postfix (see Operators), but is otherwise ignored. The following characters are considered whitespace: 
> space `(U+0020)`
> line feed `(U+000A)`
> carriage return `(U+000D)`
> horizontal tab `(U+0009)`
> vertical tab `(U+000B)`
> form feed `(U+000C)`
> null `(U+0000)`.

Comments are treated as whitespace by the compiler. Single line comments begin with `//` and continue until a line feed `(U+000A)` or carriage return `(U+000D)`. Multiline comments begin with `/*` and end with `*/`. Nesting multiline comments is allowed, but the comment markers must be balanced.

### Identifiers
Identifiers begins with charactor group:
> `a` ~ `z` , `A` ~ `Z`, `_` , `CJK_CHARS`

where `CJK_CHARS` is a collective grounp for the Chinese, Japanese, and Korean charactors.
>  `(u2E80)`~ `(u2FD5)`,`(u3190)`~ `(u319f)`,`(u3400)`~ `(u4DBF)`,`(u4E00)`~ `(u9FCC)`,`(uF900)`~ `(uFAAD')`

 After the first character, digits characters are allowed.
### Keywords and Punctuation
|**string**|**number**|**bool**||||
|:-:|:-:|:-:|:-:|:-:|:-:|
|**int**|**short**|**long**|**float**|
|**uint**|**ushort**|**ulong**|**double**|
|**true**|**false**|
|**if**|**else**|**elif**|**for**|**while**|**repeat**|
|**switch**|**case**|**default**|
|**let**|**var**|**func**|**dfunc**|**kernal**||
|**return**|**continue**|**break**|**try**|**catch**|**throw**|
|**import**|**typedef**|**extension**|**operator**|**extern**||
|**struct**|**class**|**enum**|**interface**|||
|**new**|**get**|**set**|**init**|**deinit**||
|**sizeof**|**print**|||||
|**in**|**from**|**to**|||

### Literals

#### Number
a number can be defined as
```js
1               // Integer
1.22             // Decimal
0x00000001
0X00000001      // Hexadecimal
017             // Octal
10e3            // scientific
```

#### String
```js
"hello,world"
"你好，世界"
"👋,🌎"
```
#### Bool 
`true` and `false`

### Operators
#### Scope Operators
> `(` `)`\
> `[` `]`
#### Unary Operators
###### Prefix
> `-` `!`  `~`
###### Postfix
> `++` `--`
#### Binary Operators
>`.`\
>`*` `/` `%`\
>`+` `-`\
>`<<` `>>` \
>`>` `<` `>=` `<=`\
>`&` `^` `|`\
>`==` `!=`\
>`||` `&&`\
>`? :`\
>`=`
>`/=` ` *= ` ` %=` `+=` `-=` `<<=` `>>=` `&=` `^=` `|=`       

