#include "veins/modules/mobility/traci/TraCIPenetration.h"
#include <cmath>
#include <stdexcept>

namespace Veins {

TraCIPenetration::TraCIPenetration() : penetrationRate(1.0)
{
}

bool TraCIPenetration::becomesUnequipped(const std::string& id, std::size_t equipped)
{
	using std::fabs;

	bool unequipped = false;
	const double total = equipped + unEquippedHosts.size() + 1;
	const double optionUnequipped = equipped / total;
	const double optionEquipped = (equipped + 1) / total;

	// option closer to target penetration rate wins
	if (fabs(optionUnequipped - penetrationRate) < fabs(optionEquipped - penetrationRate)) {
		unEquippedHosts.insert(id);
		unequipped = true;
	}

	return unequipped;
}

bool TraCIPenetration::removeIfUnequipped(const std::string& objectId)
{
	bool removed = false;
	std::set<std::string>::const_iterator found = unEquippedHosts.find(objectId);
	if (found != unEquippedHosts.end()) {
		unEquippedHosts.erase(found);
		removed = true;
	}
	return removed;
}

bool TraCIPenetration::isUnequipped(const std::string& objectId) const
{
	return unEquippedHosts.find(objectId) != unEquippedHosts.end();
}

void TraCIPenetration::setTargetPenetration(double value)
{
	if (value < 0.0 || value > 1.0) {
		throw std::domain_error("penetration rate has to be [0.0; 1.0]");
	}
	penetrationRate = value;
}

} // namespace Veins
