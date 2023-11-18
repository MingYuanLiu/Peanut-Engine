#pragma once

#include "Peanut/Core.h"
#include "Peanut/Event/Event.h"

namespace Peanut_Engine 
{
    class PE_API Layer
    {
    public:
        Layer(const std::string& layer_name = "Layer");
        virtual ~Layer();

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate() {}
        virtual void OnEvent(Event& event) {}

        inline const std::string& GetName() const { return m_DebugName; }

    private:
        std::string m_DebugName;
    };
}