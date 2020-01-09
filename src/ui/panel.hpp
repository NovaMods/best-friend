#pragma once

#include "../ec/component.hpp"

/*!
 * \brief Interface to a UI panel
 */
class Panel : public nova::ec::Component {
public:
    explicit Panel(nova::ec::Entity* owner_in);

    Panel(const Panel& other) = default;
    Panel& operator=(const Panel& other) = default;

    Panel(Panel&& old) noexcept = default;
    Panel& operator=(Panel&& old) noexcept = default;

    ~Panel() override = default;

    /*!
     * \brief Draws this panel using the `draw` template method. May be overriden to perform additional work, but you'll need to either call
     * this method or call `draw` yourself
     */
    void tick(double delta_time) override;

    /*!
     * \brief Draws the panel into whatever panel-drawing system we have
     */
    virtual void draw() = 0;
};
