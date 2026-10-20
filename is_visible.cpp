// Видно, если станция выше 10°, у наблюдателя сумерки (солнце ниже -6°)
// и сама станция на солнце.
bool is_visible(double station_elevation, double sun_elevation, bool station_in_sunlight)
{
	if (station_elevation <= 10) {
		return false;
	}
	if (sun_elevation >= -6) {
		return false;
	}
	if (station_in_sunlight == false) {
		return false;
	}
	return true;
}
