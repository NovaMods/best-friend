#pragma once

namespace nova::ec {
	class Entity;

	/*!
	 * \brief Interface for your custom components
	 * 
	 * This is similar to Unity's MonoBehaviour, NOT to UE4 ActorComponents
	 * 
	 * When you make a class that implements Component, all its constructors _must_ take the owning entity as their 
	 * first parameter. 
	 */
	class Component
	{
	public:
		Entity* owner;

		/*!
		 * \brief Initializes this component, whatever that ends up meaning
		 * 
		 * When you implement Component for your custom components, you MUST provide a constructor with Entity* as the 
		 * type of its first parameter. You may have as many other parameters as you wish. The construct MUST call this
		 *  constructor, or nothing will work properly
		 */
		Component(Entity* owner);

		Component(const Component& other) = delete;
		Component& operator=(const Component& other) = delete;

		virtual ~Component() = default;

		/*!
		 * \brief Perform whatever per-frame work that this component performs
		 */
        virtual void tick(double /* delta_time */) {}
	};
}
