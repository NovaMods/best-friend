#pragma once 

/*!
 * \brief Interface to a UI panel
 */
class Panel {
public:
    virtual ~Panel() = default;
    /*!
     * \brief Draws the panel into whatever panel-drawing system we have
     */
    virtual void draw() = 0;
};