#include "smashorpass/app/Application.hpp"

#include "smashorpass/layer/GameLayer.hpp"
#include "smashorpass/layer/UILayer.hpp"

int main()
{
    sop::Application application;
    application.PushLayer<sop::UILayer>();
    application.PushLayer<sop::GameLayer>();
    return application.Run();
}
