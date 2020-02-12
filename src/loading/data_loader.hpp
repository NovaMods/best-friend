#pragma once
class World;

namespace nova {
    namespace renderer {
        class NovaRenderer;
    }
} // namespace nova

namespace nova::bf {
    struct LoadTrainEvent;

    class DataLoader {
    public:
        DataLoader(World& world, renderer::NovaRenderer& renderer);

        void load_train(const LoadTrainEvent& event);

    private:
        World& world;
        renderer::NovaRenderer& renderer;
    };
} // namespace nova::bf
