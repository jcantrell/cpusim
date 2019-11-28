#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unordered_map>
#include <iostream>

#include "ui/UI.h"
#include "model/cpu/cpu.h"
#include "model/subleq/subleq.h"
#include "model/mmix/mmix.h"
#include "model/loader/Loader.h"

class TextUI : public UI {
  private:
    Loader loader;
	public:
		int user_loop();
};
