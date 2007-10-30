#include "PositifUtil.h"

int PositifUtil::nr_dims = 2;
bool PositifUtil::use_confs = false;

FLOAT PositifUtil::distance(Position a, Position b)
{
	FLOAT sumsqr = 0;
	for (int i = 0; i < nr_dims; i++) {
		sumsqr += (a[i] - b[i]) * (a[i] - b[i]);
	}
	return sqrt(sumsqr);
}

FLOAT PositifUtil::savvides_minmax(int n_pts, FLOAT ** positions,
				    FLOAT * ranges, FLOAT * confs, int target)
{
	//
	//  Savvides' algorithm
	//
	Position min, max;

	// Find the max-min and min-max in each dimension.
	for (int i = 0; i < n_pts; i++)
		for (int j = 0; j < nr_dims; j++) {
			if (positions[i][j] - ranges[i] > min[j] || i == 0)
				min[j] = positions[i][j] - ranges[i];
			if (positions[i][j] + ranges[i] < max[j] || i == 0)
				max[j] = positions[i][j] + ranges[i];
		}

	// Store the result (avg of min and max)
	for (int i = 0; i < nr_dims; i++) {
		positions[n_pts][i] = (min[i] + max[i]) / 2;
	}

	FLOAT residu = 0;
	FLOAT sum_c = 0;
	for (int j = 0; j < n_pts; j++) {
		FLOAT c = (confs == NULL ? 1 : confs[j]);
		residu +=
			c * fabs(ranges[j] -
				 distance(positions[n_pts], positions[j]));
		sum_c += c;
	}
	residu /= sum_c;

	return residu;
}

FLOAT PositifUtil::triangulate(int n_pts, FLOAT ** positions,
				FLOAT * ranges, FLOAT * confs, int target)
{
	FLOAT dop;

	if (n_pts <= nr_dims)
		return -1;

#ifndef NDEBUG
	if (confs != NULL) {
		EV << "confs:";
		for (int j = 0; j < n_pts; j++) {
			EV << " " << confs[j];
		}
		EV << "\n";
	}
#endif

	::params.dim = nr_dims;
	::params.alg_sel = 0;
	::params.conf_mets = use_confs && (confs != NULL);
	if (!::triangulate(&::params, n_pts, positions, ranges, confs,
			   positions[n_pts], &dop)) {
#ifndef NDEBUG
		EV << ": FAILED TRIANGULATION\n";
#endif
		return -1;
	}
	// triangulate() used the last point to linearize the equations.
	// Use the estimated position for an extra equation so all inputs are
	// treated equally.
	ranges[n_pts] = 0;
	::triangulate(&::params, n_pts + 1, positions, ranges, confs,
		      positions[n_pts], &dop);
#ifndef NDEBUG
	EV << ": " << pos2str(positions[n_pts]);
#endif

	FLOAT residu = 0;
	FLOAT sum_c = 0;
	for (int j = 0; j < n_pts; j++) {
		FLOAT c = (confs == NULL ? 1 : confs[j]);
		residu +=
			c * fabs(ranges[j] -
				 distance(positions[n_pts], positions[j]));
		sum_c += c;
	}
	residu /= sum_c;

#ifndef NDEBUG
	EV << " RESIDU = " << residu
// 	   << " ERR = "
// 	   << 100 * getPosition().distance(positionToCoord(positions[n_pts], nr_dims)) / range << "%"
	   << endl;
#endif
	return residu;
}

FLOAT PositifUtil::hoptriangulate(int n_pts, FLOAT ** positions,
				   FLOAT * ranges, int target)
{
	FLOAT est_R;

	if (n_pts <= nr_dims + 1)
		return false;

	::params.dim = nr_dims;
	::params.alg_sel = 0;
	::params.conf_mets = false;
	if (!::hoptriangulate(&::params, n_pts, positions, ranges, NULL,
			      positions[n_pts], &est_R)) {
#ifndef NDEBUG
		EV << ": FAILED TRIANGULATION\n";
#endif
		return -1;
	}
#ifndef NDEBUG
	EV << ":A " << pos2str(positions[n_pts]);
#endif

	// Run sanity check
	for (int i = 0; i < n_pts; i++)
		ranges[i] *= est_R;
	FLOAT correction;
	::hoptriangulate(&::params, n_pts + 1, positions, ranges, NULL,
			 positions[n_pts], &correction);
#ifndef NDEBUG
	EV << ":B " << pos2str(positions[n_pts]);
#endif

	return (correction < .9 || correction > 1.1 ? -1 : correction * est_R);
}
