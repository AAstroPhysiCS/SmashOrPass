#include "smashorpass/app/Application.hpp"
#include "smashorpass/layer/DebugLayer.hpp"

int main() {
    sop::Application application;
    application.PushOverlay<sop::DebugLayer>();
    return application.Run();
}