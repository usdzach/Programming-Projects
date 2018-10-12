/* Author: Zach Fukuhara <zfukuhara@sandiego.edu>
 *
 * Description:  Helper class for FlightBookingSimulator.java which
 * represents a city with the name, previous city in the flight path,
 * cost to get to this city, and whether or not the city has been previously visited.
 */

import java.util.ArrayList;
public class FlightSimCity
{
    private String cityName;
    private City parentCity;  // the previous city in the flight path (ex. Paris to Chicago.  Paris is parent to Chicago.)
    private int costToGetHere;
    private boolean visited;
    private ArrayList<Flight> flights;

    public City(String newCity)
    {
        cityName = newCity;
        parentCity = null;
        costToGetHere = 1000000000;     //upperbound for less than comparison
        visited = false;
        flights = new ArrayList<Flight>();
    }

    public City(String newCity, int newCost)
    {
        cityName = newCity;
        parentCity = null;
        costToGetHere= newCost;
        visited = false;
        flights = new ArrayList<Flight>();
    }

    //-----------------------Setters-------------------------

    public void setCostToGetHere(int newCost)
    {
        costToGetHere = newCost;
    }

    public void addFlight(City destination, int ticketPrice)
    {
        flights.add(new Flight(destination, ticketPrice));
    }

    public void setVisited(boolean bool) {
        visited = bool;
    }

    public void setParent(City newParent)
    {
        parentCity = newParent;
    }

    //-----------------------Getters-------------------------

    public String getCityName()
    {
        return cityName;
    }

    public int getCostToGetHere()
    {
        return costToGetHere;
    }

    public boolean wasVisited()
    {
        return visited;
    }

    public ArrayList<Flight> getFlights()
    {
        return flights;
    }

    public City getParent()
    {
        return parentCity;
    }
}
