# Types

#### Import Declaration
> **import** `package`
#### Constant Declaration
> **let** `name` (**:** `type`)<sub>opt</sub> **=** `expression`
#### Variable Declaration
> **var** `name` (**:** `type`)<sub>opt</sub> **=** `expression`
#### Computed Variables and Computed Properties

>**get** **{** `statements` **}**\
>**set** **{** `statements` **}**

>**get** `=>` `statements` \
>**set** `=>` `statements`
#### TypeDef
> **typedef** `alias` **=** `type` 

#### Function Declaration
> `function_type`  `name` **(**  <`type` `name`><sub>rpt opt</sub>  **)** <**:** `type`><sub>opt</sub> `function_body`

`function_type` :
>**func**   ,   **dfunc**   ,  **kernal**

`function_body`:
> **{** `statements` **}**\
or\
> **=** `statement`
##### Initializer Declaration
> **init** **(**  <`type` `name`><sub>rpt opt</sub>  **)** <**:** `type`><sub>opt</sub> `function_body`

##### Deinitializer Declaration
> **deinit** **(**  <`type` `name`><sub>rpt opt</sub>  **)** <**:** `type`><sub>opt</sub> `function_body`

#### Enumeration Declaration
>**enum** `name`
> **{** `name` **,** `name`<sub>rpt</sub> ... **}**
#### Structure Declaration
> **struct**  `name` **:** `inherit` **{** `declarations` **}**
#### Class Declaration
> **class**  `name` **:** `inherit_class` **{** `declarations` **}**
#### Protocol(Interface) Declaration
> **interface**  `name` **:** `inherit_interface` **{** `declarations` **}**
#### Extension Declaration
> **extension**  `type` **{** declarations **}**
#### Extern Declaration
> **extern**  **func**  `name` **(**  <`type` `name`><sub>rpt opt</sub>  **)** <**:** `type`><sub>opt</sub>

#### Subscript Declaration?

#### Operator Declaration
> **operator**  `op`  **(**  <`type` `name`><sub>rpt opt</sub>  **)** **:** `type` `function_body`