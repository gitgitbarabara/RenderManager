#include "Layer.hpp"

BlendFunc::BlendFunc(const int _signature, const GLenum _srcValue, const GLenum _dstValue) : Layer(_signature), srcValue(_srcValue), dstValue(_dstValue){

}
BlendFunc::~BlendFunc() {

}
void BlendFunc::Render() {
    glBlendFunc(srcValue, dstValue);
}

Stencil::Stencil(const int signature, GLenum func, GLint ref, GLint mask, GLenum fail, GLenum zfail, GLenum zpass) :
        Layer(signature),
        func(func),
        ref(ref),
        mask(mask),
        fail(fail),
        zfail(zfail),
        zpass(zpass){

}

Stencil::~Stencil() {

}

void Stencil::Render() {
    glStencilFunc(func, ref, mask);
    glStencilOp(fail, zfail, zpass);
}