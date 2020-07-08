#include "GLCore.h"
#include "GameLayer.h"

using namespace GLCore;

class Example : public Application
{
public:
	Example()
		: Application("Cities Bad")
	{
		PushLayer(new GameLayer());
	}
};

int main()
{
	std::unique_ptr<Example> app = std::make_unique<Example>();
	app->Run();
}