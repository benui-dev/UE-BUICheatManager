# BUI Cheat Manager


## Usage

1. Create a C++ subclass of UBUICheatManagerBase:

```cpp
#pragma once

#include "BUICheatManagerBase.h"
#include "MyCheatManager.generated.h"

UCLASS( meta = ( CheatPrefix = "mygame." ) )
class UMyCheatManager : public UBUICheatManagerBase
{
	GENERATED_BODY()

public:
	UFUNCTION( exec, meta = ( Cheat = "car.spawn" ) )
		void SpawnCar( int32 num );
};
```
2. Add cheat functions as you would normally, but add the tag "Cheat" with
   a shortcut name.
3. Create a Blueprint subclass of your Cheat Manager. This step is
   **required**. The shortcut names only work by using the asset tags registry.
4. Set your PlayerController to use the Blueprint subclass of your Cheat
   Manager.
5. Play the game and press tilde \` to bring up the console. Your cheats will
   be listed in Cyan with the prefix set in CheatPrefix.


## Thanks

Huge thanks to Geordie Hall at Bit Dragon for the bulk of this technique.


