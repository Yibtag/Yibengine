#include "client.h"

int main() {
	yib::Client client = yib::Client("Yibengine", 1280, 720);
	if (!client.success) {
		return 1;
	}

	client.Run();

	return 0;
}