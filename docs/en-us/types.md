# Types

There are three basic type and x complex type in dragonfly.
> **Basic Type**\
> type → [number-type](#Number-type)\
> type → [string-type](#String-type)\
> type → [bool-type](#Bool-type)\
> **Complex Type Type**
> 

### Basic types

> `number` `string` `bool`\
> `int` `short` `long` `float` `double`\
> `uint` `ushort` `ulong`\
> `byte`

### Tuple
```
let x: [string, number]
```
### Array

**[** `value` **,** `value` **]**


```js
let a:number[]=1
```

### Tensor
Multidimensional Arrays or Matrices

**[** `size` **,**  `size` **,**  `size`  **]**

example:
```js
let a:number[1,1] = [[1],[1]]
let b:number[1,1] = [1,1]

let c:number[3,3,3] = [[1,1,1],
                      [1,1,1],
                      [1,1,1]]

let c:number[3,,3] = [[1,1,1],
                     [1,1,1,1,1,1,1,1,1],
                     [1,1,1]]
```

### Dictionary
**[** `key` : `value` **,** `key` : `value` **]**

### Function
funtion is a type in dragonfly

```js
func f2()=2

func f1(int a)=a*2

let f = f2

let f: (int)=>int =f1

```