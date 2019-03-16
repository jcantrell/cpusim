#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unordered_map>
#include <iostream>

#include "ui/UI.h"
#include "model/cpu.h"
#include "model/subleq.h"
#include "model/mmix.h"

class TextUI : public UI {
	public:
		int user_loop();
};
