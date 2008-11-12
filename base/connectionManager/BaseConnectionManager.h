#ifndef BASECONNECTIONMANAGER_H_
#define BASECONNECTIONMANAGER_H_

#include "BaseModule.h"
#include "NicEntry.h"

class ChannelAccess;

/**
 * @brief Module to control the channel and handle all connection
 * related stuff
 *
 * The central module that coordinates the connections between all
 * nodes, and handles dynamic gate creation. BasicConnectionManager therefore
 * periodically communicates with the ChannelAccess modules
 *
 * You may not instantiate BasicConnectionManager!
 * Use ConnectionManager instead.
 *
 * @ingroup ConnectionManager
 * @author Steffen Sroka, Daniel Willkomm, Karl Wessel
 * @sa ChannelAccess
 */
class BaseConnectionManager : public BaseModule
{
private:
	/**
	 * Represents a position inside a grid.
	 * This class provides some converting functions from a Coord
	 * to a GridCoord.
	 */
	class GridCoord
	{
	public:
		static const int UNDEFINED = 0;
		int x;
		int y;
		int z;
		bool use2D;


		/**
		 * Initialize this GridCoord with the origin.
		 * Creates a 3-dimensional coord.
		 */
		GridCoord()
			:x(0), y(0), z(0), use2D(false) {};

		/**
		 * Initialize a 2-dimensional GridCoord with x and y.
		 */
		GridCoord(int x, int y)
			:x(x), y(y), z(UNDEFINED), use2D(true) {};

		/**
		 * Initialize a 3-dimensional GridCoord with x, y and z.
		 */
		GridCoord(int x, int y, int z)
			:x(x), y(y), z(z), use2D(false) {};

		/**
		 * Simple copy-constructor.
		 */
		GridCoord(const GridCoord& o) {
			x = o.x;
			y = o.y;
			z = o.z;
			use2D = o.use2D;
        }

		/**
		 * Creates a GridCoord from a given Coord by dividing the
		 * x,y and z-values by "gridCellWidth".
		 * The dimension of the GridCoord depends on the Coord.
		 */
        GridCoord(const Coord& c, double gridCellWidth = 1.0) {
            x = static_cast<int>(c.getX() / gridCellWidth);
            y = static_cast<int>(c.getY() / gridCellWidth);
            z = static_cast<int>(c.getZ() / gridCellWidth);
            use2D = c.is2D();
        }

        std::string info() const {
			std::stringstream os;
			if (use2D) {
				os << "(" << x << "," << y << ")";
			} else {
				os << "(" << x << "," << y << "," << z << ")";
			}
			return os.str();
		}

		friend bool operator==(const GridCoord& a, const GridCoord& b) {
			return a.x == b.x && a.y == b.y && a.z == b.z;
		}

		friend bool operator!=(const GridCoord& a, const GridCoord& b) {
			return !(a==b);
		}

		GridCoord operator=(const GridCoord& a) {
			x = a.x;
			y = a.y;
			z = a.z;
			use2D = a.use2D;
			return *this;
		}
	};

	/**
	 * Represents an unsorted set of GridCoords with minimalistic functions.
	 * It is a workaround because c++ doesn't come with an unsorted set.
	 */
	class CoordSet {
	protected:
		std::vector<GridCoord*> data;
		unsigned maxSize;
		unsigned size;
		unsigned current;

	protected:

		/**
		 * Tries to insert a GridCoord at the specified position.
		 * If the same Coord already exists there nothing happens.
		 * If the an other Coord already exists there calculate
		 * a new Position to isnert end recursively call this Method again.
		 * If the spot is empty the Coord is inserted.
		 */
		void insert(const GridCoord& c, unsigned pos) {
			if(data[pos] == 0) {
				data[pos] = new GridCoord(c);
				size++;
			} else {
				if(*data[pos] != c) {
					insert(c, (pos + 2) % maxSize);
				}
			}
		}

	public:
		/**
		 * Initializes the set (hashtable) with the a specified size.
		 */
		CoordSet(unsigned sz)
			:maxSize(sz), size(0), current(0)
		{
			data.resize(maxSize);
		}

		/**
		 * Delete every created GridCoord
		 */
		~CoordSet() {
			for(unsigned i = 0; i < maxSize; i++) {
				if(data[i] != 0) {
					delete data[i];
				}
			}
		}

		/**
		 * Adds a GridCoord to the set. If a GridCoord with the same
		 * value already exists in the set nothing happens.
		 */
		void add(const GridCoord& c) {
			unsigned hash = (c.x * 10000 + c.y * 100 + c.z) % maxSize;
			insert(c, hash);
		}

		/**
		 * Returns the next GridCoord in the set.
		 * You can interate through the set only one time with this function!
		 */
		GridCoord* next() {
			for(;current < maxSize; current++) {
				if(data[current] != 0) {
					return data[current++];
				}
			}
			return 0;
		}

		/**
		 * Returns the number of GridCoords currently saved in this set.
		 */
		unsigned getSize() { return size; }

		/**
		 * Returns the maximum number of elements which can be stored inside
		 * this set. To prevent collisions the set should never be more
		 * than 75% filled.
		 */
		unsigned getmaxSize() { return maxSize; }
	};

protected:
	typedef std::map<int, NicEntry*> NicEntries;

	NicEntries nics;

	/** @brief Set debugging for the basic module*/
	bool coreDebug;

	/** @brief Does the ConnectionManager use sendDirect or not?*/
	bool sendDirect;

	/** @brief Stores the size of the playground.*/
	const Coord* playgroundSize;

	/** @brief the biggest interference distance in the network.*/
	double maxInterferenceDistance;

	/** @brief Square of maxInterferenceDistance
	 * cache a value that is often used
	 */
	double maxDistSquared;

	/** @brief Stores the useTorus flag of the WorldUtility */
	bool useTorus;


	typedef std::vector<NicEntries> RowVector;
	typedef std::vector<RowVector> NicMatrix;
    typedef std::vector<NicMatrix> NicCube;

	/** @brief Registry of all Nics
     *
     * This matrix keeps all Nics according to their position.  It
     * allows to restrict the position update to a subset of all nics.
     */
    NicCube nicGrid;

    /**
     * Distance that helps to find a node under a certain
     * position. Can be larger then @see maxInterferenceDistance to
     * allow nodes to be placed into the same square if the playground
     * is too small for the grid speedup to work.
	 */
    double findDistance;

    /**
     * the dimension of the grid
     */
    GridCoord gridDim;

private:
	/** @brief Manages the connections of a registered nic. */
    void updateNicConnections(NicEntries& nmap, NicEntry* nic);

    /**
     * check connections of a nic in the grid
     */
    void checkGrid(GridCoord& oldCell,
                   GridCoord& newCell,
                   int id);

    /**
     * Calculates the corresponding cell of a coordinate.
     */
    GridCoord getCellForCoordinate(const Coord& c);

    /**
     * Returns the NicEntries of the cell with specified
     * coordinate.
     */
    NicEntries& getCellEntries(GridCoord& cell);

	/**
	 * If the value is outside of its bounds (zero and max) this function
	 * returns -1 if useTorus is false and the wrapped value if useTorus is true.
	 * Otherwise its just returns the value unchanged.
	 */
    int wrapIfTorus(int value, int max);

	/**
	 * Adds every direct Neighbor of a GridCoord to a union of coords.
	 */
    void fillUnionWithNeighbors(CoordSet& gridUnion, GridCoord cell);
protected:

	/**
	 * @brief Calculate interference distance
	 * This method has to be overridden by any derived class.
	 */
	virtual double calcInterfDist() = 0;

	/**
	 * @brief This method is called by "registerNic()" after the nic has been
	 * registered. That means that the NicEntry for the nic has already been
	 * created and added to nics map.
	 *
	 * You better know what you are doing if you want to override this
	 * method. Most time you won't need to.
	 *
	 * See ConnectionManager::registerNicExt() for an example.
	 *
	 * @param nicID - the id of the NicEntry
	 */
	virtual void registerNicExt(int nicID);

	/**
	 * @brief Updates the connections of the nic with "nicID".
	 *
	 * This method is called by "updateNicPos()" after the
	 * new Position is stored in the corresponding nic.
	 *
	 * Most time you won't need to override this method.
	 *
	 * @param nicID the id of the NicEntry
	 * @param oldPos the old position of the nic
	 * @param newPos the new position of the nic
	 */
	virtual void updateConnections(int nicID, const Coord* oldPos, const Coord* newPos);

public:

	/**
	 * @brief Constructor
	 **/
	//Module_Class_Members(BaseConnectionManager, BaseModule, 0);


	virtual ~BaseConnectionManager();

	/**
	 * @brief Reads init parameters and calculates a maximal interfence
	 * distance
	 **/
	virtual void initialize(int stage);

	/**
	 * @brief Registers a nic to have its connections managed by ConnectionManager.
	 *
	 * If you want to do your own stuff at the registration of a nic see
	 * "registerNicExt()".
	 */
	bool registerNic(cModule* nic, ChannelAccess* chAccess, const Coord* nicPos);

	/** @brief Updates the position information of a registered nic.*/
	void updateNicPos(int nicID, const Coord* newPos);

	/** @brief Returns the ingates of all nics in range*/
	const NicEntry::GateList& getGateList( int nicID);

	/** @brief Returns the ingate of the with id==targetID, or 0 if not in range*/
	const cGate* getOutGateTo(const NicEntry* nic, const NicEntry* targetNic);
};

#endif /*BASECONNECTIONMANAGER_H_*/
