#ifndef VEINS_WORLD_TRACI_TRACIPENETRATION_H
#define VEINS_WORLD_TRACI_TRACIPENETRATION_H

#include <set>
#include <string>

namespace Veins {

/**
 * Helper class for managing a target penetration rate in a scenario
 * For this purpose it tracks the unequipped nodes, whereas TraCINodeManager tracks the equipped ones
 */
class TraCIPenetration
{
	public:
		TraCIPenetration();

		/**
		 * Decide if node shall be equipped with module
		 * \param id node identifier
		 * \param equipped number of currently equipped nodes
		 * \return true if node shall not be equipped
		 */
		bool becomesUnequipped(const std::string& id, std::size_t equipped);

		/**
		 * Removes node from set of unequipped nodes
		 * \param id node identifier
		 * \return true if node has been element of set
		 */
		bool removeIfUnequipped(const std::string& id);

		/**
		 * Test if a node is equipped
		 * \param id node identifier
		 * \return true if node is unequipped
		 */
		bool isUnequipped(const std::string& id) const;

		/**
		 * Set target penetration rate which shall be maintained
		 * \param value penetration rate [0.0; 1.0]
		 */
		void setTargetPenetration(double value);

	private:
		double penetrationRate;
		std::set<std::string> unEquippedHosts;
};

} // namespace Veins

#endif
