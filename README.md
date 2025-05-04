# GuardAndTowesr
symbolic KI to the game Guards and Towers


## Variable and Function Comparison

| Feature                     | **Static Member**                | **Non-Static Member**             | **Free Static Variable**          | **Free Non-Static Variable**   | **Local (Stack) Variable**               |
| --------------------------- | -------------------------------- | --------------------------------- | --------------------------------- | ------------------------------ | ---------------------------------------- |
| **Belongs to**              | Class                            | Instance                          | File/Translation unit             | Global namespace               | Function                                 |
| **Access syntax**           | `Class::var`                     | `obj.var`                         | Direct name (within file)         | Direct name (anywhere)         | Direct name (in function)                |
| **Memory location**         | `.data` / `.bss` (static memory) | Heap or stack (depends on object) | `.data` / `.bss`                  | `.data` / `.bss`               | Stack frame                              |
| **Lifetime**                | Whole program                    | Tied to object lifetime           | Whole program                     | Whole program                  | Created & destroyed on each call         |
| **Shared across instances** | ✅ Yes                            | ❌ No                              | ✅ Yes                             | ✅ Yes                          | ❌ No                                     |
| **Access speed**            | ✅ Fast (fixed addr)              | 🟡 Fast (needs `this`)            | ✅ Fast (fixed addr)               | ✅ Fast (fixed addr)            | ✅✅ Very fast (can be register-optimized) |
| **Cache-friendliness**      | ✅ (if reused often)              | 🟡 Depends on object locality     | ✅ (if reused in same TU)          | ✅ (global reuse)               | 🟡 Depends — reloads every call          |
| **Initialization cost**     | Once                             | Per object                        | Once                              | Once                           | Every function call                      |
| **Scope / Namespace**       | Class namespace                  | Class namespace                   | File-local only                   | Global namespace               | Function scope                           |
| **Ideal use**               | Shared class data / settings     | Per-object state                  | Internal constants, helpers       | App-wide config / shared state | Temporary / local computation            |
| **Thread safety**           | ❌ Needs care (shared state)      | ✅ (per object)                    | ❌ Needs care (shared across file) | ❌ Global mutable state         | ✅ Safe unless using static local         |


--> local variable are fastest, but are recreated on every function call --> need to be cached again.\
Candiates are static members and free static variables

| Feature                  | Free `static` variable  | Static class member                         |
| ------------------------ | ----------------------- | ------------------------------------------- |
| Compiler visibility      | High (local to file)    | Medium (visible across TU)                  |
| Linkage                  | Internal (not exported) | External (if in header)                     |
| Cache placement          | Close to related code   | Maybe far (depends on layout)               |
| Alias analysis safety    | ✅ Very safe             | 🟡 Less safe (could be externally modified) |
| Optimization opportunity | ✅ High                  | 🟡 Slightly lower                           |
| Use in hot loop          | ✅ Ideal                 | ✅ OK                                        |

--> Using free static variables and functions 

## Varaibles Usage

we almost entirely work with free static variables, 
- Locals have to be cached after each function call --> avoid local variables
- cache friendliest and compiler optimization friendliest option
- shared stayed through the whole board.cpp file, keep track of right usage

## Board representation
 The board is (at the moment) in the first 49 LSB of the std::uint64_t. \
 Stackheight is **!sometimes!** encoded in the first 3 MSB bits 111-> height 7, 001-->height 1, etc. \
 The normal Figure are not seperatedly saved / do not own their own uint64_t. \
 ### figuresB/ figuresR:
 - Array containg 1 uint64_t for every possible height, eg length 7
 - Example: A figure of height 3 has its position marked in the first 3 uint64s of the array. Its positional bit is set in figuresX[0], figuresX[1], figuresX[2]

 ## Move Generator
 - loops over the 4 dimesnions in 2 seperate loops one loop for left shifts, the other for right shifts
 - within the dimension loop --> loop over the step size

