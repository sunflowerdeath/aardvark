#include "android_app.hpp"
#include "android_binary_channel.hpp"

namespace aardvark {

AndroidApp::AndroidApp(jobject activity, jobject platform_channel, int width,
                       int height)
    : activity(activity), width(width), height(height) {
    auto binary_channel =
        aardvark::AndroidBinaryChannel::get_native_channel(platform_channel);
    auto system_channel = new aardvark::MessageChannel<json>(
        binary_channel, aardvark::JsonCodec::get_instance());
    register_channel("system", system_channel);
    document = std::make_shared<Document>();
}

void AndroidApp::update() {
    document->paint();
}

}  // namespace aardvark
