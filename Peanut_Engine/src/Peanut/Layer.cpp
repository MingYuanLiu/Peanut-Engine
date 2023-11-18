#include "pepch.h"
#include "Layer.h"

namespace Peanut_Engine
{
    Layer::Layer(const std::string& layer_name)
        : m_DebugName(layer_name)
    {}

    Layer::~Layer()
    {
        
    }
}