# reflstruct

C++20 Reflection Library.

* Access member variables types and values, both at compile-time and at runtime.
* Golang style member variables string annotations parsed at compile-time to implement marshalers.

## Example

Definition of a `service` structure holding typical network service configuration elements.
The macros `TREZZ_REFL...` add reflection information.

```cpp
#include "trezz/reflstruct.h"
#include <string>

struct service {
  int port{ 80 };
  std::string host{};
  std::string db_url{};
  
  TREZZ_REFLSTRUCT_BEGIN(service)
  TREZZ_REFLMEMBER(port, "envconfig:name=MYSERVICE_PORT")
  TREZZ_REFLMEMBER(host, "envconfig:name=MYSERVICE_HOST,required")
  TREZZ_REFLMEMBER(db_url, "envconfig:name=MYSERVICE_DB_URL,required")
  TREZZ_REFLSTRUCT_END
};
```

Get the `reflstruct` of a type:

```cpp
service s{};
trezz::reflstruct rs = service::make_trezz_reflstruct(s);

const service cs{};
const trezz::reflstruct crs = service::make_trezz_reflstruct(cs);
```

Iterate on each reflmember of a reflstruct:

```cpp
crs.each([](const auto& member) {
  // ...
});
```

Populate members with values taken from the environment using `trezz::envconfig`, leveraging the member annotations:

```cpp
#include "trezz/envconfig.h"

// Environment with:
//    MYSERVICE_HOST=example.com
//    MYSERVICE_DB_URL=example.com/db

service s{};

trezz::envconfig::process(s);

assert(s.port == 80); // MYSERVICE_PORT not found in the environment. Default value kept.
assert(s.host == "example.com");
assert(s.db_url == "example.com/db");

```
