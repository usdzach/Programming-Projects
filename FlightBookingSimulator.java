/* Author: Zach Fukuhara <zfukuhara@sandiego.edu>
 *
 * Description:  This program implements of slightly modified Dijkstra's Algorithm
 * to simulate a Flight Booking Website.  Users enter a textfile with a list of cities,
 * along with a textfile listing the flights available between those cities.  From there,
 * users can enter a departure city and destination, and the program will out the
 * cheapest flight path between those cities.  Note: The program could be modified to
 * output the fastest flight path, assuming the user supplied information about flights
 * durations in an input textfile.
 */

import java.io.*;
import java.util.*;
public class FlightBookingSimulator
{
    public static void main(String[] args)
    {
        //---------------------------------------------------------Read In Test Files--------------------------------------------------------
        Scanner keyboard = new Scanner(System.in);

        System.out.println("Please enter a cityFile in the format '________.txt'");
        String cityFile = keyboard.nextLine().trim();
        System.out.println("Please enter a flightFile in the format '________.txt'");
        String flightFile = keyboard.nextLine().trim();
        System.out.println("Welcome to USAir!");
        while(true)
        {
            try
            {
                //--------------------------------------Initialize Flight Map & Read In User Flight Info--------------------------------------

                BufferedReader cityBuf = new BufferedReader(new FileReader(cityFile));
                BufferedReader flightBuf = new BufferedReader(new FileReader(flightFile));

                System.out.println("\nWould you like to make flight reservations? (Y/N)");
                String userResponse = keyboard.nextLine().trim();
                if(!(userResponse.equalsIgnoreCase("Y") || userResponse.equalsIgnoreCase("Yes")))
                {
                    System.out.println("Goodbye!");
                    break;
                }

                ArrayList<City> cities = new ArrayList<City>();
                StringTokenizer temp;
                String tempDepartureCity, tempDestination;
                String userDepartureCity, userDestination;
                int tempTicketPrice;

                // Read in the cities
                while(cityBuf.ready())
                    cities.add(new City(cityBuf.readLine()));
                
                // Read in the departing flights from each city
                while(flightBuf.ready())
                {
                    temp = new StringTokenizer(flightBuf.readLine(), ",");
                    tempDepartureCity = temp.nextToken();
                    tempDestination = temp.nextToken().trim();
                    tempTicketPrice = Integer.parseInt(temp.nextToken().trim());

                    int index = getCityIndex(cities, 0, cities.size() - 1, tempDepartureCity);
                    if(index != -1)
                        cities.get(index).addFlight(getCity(cities, 0, cities.size() - 1, tempDestination), tempTicketPrice);
                }

                for (int i = 0; i < cities.size(); i++) {
                    System.out.println(cities.get(i).getCityName());
                    for (int j = 0; j < cities.get(i).getFlights().size(); j++) {
                        System.out.println(cities.get(i).getCityName() + "'s flight destination: " + cities.get(i).getFlights().get(j).getDestination().getCityName());
                        System.out.println("Ticket price: " + cities.get(i).getFlights().get(j).getTicketPrice());
                    }
                }

                // Read in user's flight details (departure city & destination)
                System.out.println("Please enter the city you wish to depart from: ");
                userDepartureCity = keyboard.nextLine().trim();
                System.out.println("Please enter the city you wish to travel to: ");
                userDestination = keyboard.nextLine().trim();

                //---------------------------------------------Verify User Flight Info---------------------------------------------

                boolean checkValidDepartCity = false, checkValidDest = false;

                // Check if userDepartureCity is valid
                if(getCityIndex(cities, 0, cities.size() - 1, userDepartureCity) != -1)
                    checkValidDepartCity = true;
                if(getCityIndex(cities, 0, cities.size() - 1, userDestination) != -1)
                    checkValidDest = true;

                // If the userDepartureCity or userDestination are invalid, restart
                if(!checkValidDepartCity)
                {
                    System.out.println("Error: Invalid departure city.  Please try again.");
                    continue;
                }
                if(!checkValidDest)
                {
                    System.out.println("\nResquest is to fly from " + userDepartureCity + " to " + userDestination + ".\n"
                                             + "Sorry.  USAir does not serve " + userDestination + ".");
                    continue;
                }

                //-------------------------------------------Begin Cheapest Flight Path Algorithm-------------------------------------------

                Queue q = new Queue();
                City visitedCity, adjacentCity = null, startingCity;
                int minPrice = 0, tempPrice = 0;
                startingCity = getCity(cities, 0, cities.size() - 1, userDepartureCity);
                startingCity.setCostToGetHere(0);
                q.enqueue(startingCity);

                while(!q.isEmpty())
                {
                    visitedCity = q.dequeue();
                    visitedCity.setVisited(true);
                    for(int i = 0; i < visitedCity.getFlights().size(); i++)
                    {
                        adjacentCity = visitedCity.getFlights().get(i).getDestination();

                        if(!adjacentCity.wasVisited())
                        {
                            if(adjacentCity.getCostToGetHere() > visitedCity.getCostToGetHere() + visitedCity.getFlights().get(i).getTicketPrice())
                            {
                                adjacentCity.setCostToGetHere(visitedCity.getCostToGetHere() + visitedCity.getFlights().get(i).getTicketPrice());
                                adjacentCity.setParent(visitedCity);
                                q.enqueue(adjacentCity);
                            }
                        }
                    }
                }

                //-----------------------------------------Begin Cheapest Flight Path Reconstruction-----------------------------------------
                if(getCity(cities, 0, cities.size() - 1, userDestination).getCostToGetHere() != 1000000000)
                {
                    ArrayList<City> flightPath = new ArrayList<City>();
                    City tempCity = getCity(cities, 0, cities.size() - 1, userDestination);

                    while(tempCity.getParent() != null)
                    {
                        flightPath.add(tempCity);
                        tempCity = tempCity.getParent();
                    }
                    flightPath.add(getCity(cities, 0, cities.size() - 1, userDepartureCity));
                    System.out.println("\nResquest is to fly from " + userDepartureCity + " to " + userDestination + ".");
                    for(int i = flightPath.size() - 1; i > 0; i--)
                    {
                        System.out.println("Flight from " + flightPath.get(i).getCityName() + " to " + flightPath.get(i - 1).getCityName()
                                         + "\tCost: $" + getTicketPrice(flightPath.get(i).getFlights(), flightPath.get(i - 1)));
                    }
                    System.out.println("Total Cost................... $" + getCity(cities, 0, cities.size() - 1, userDestination).getCostToGetHere());
                }
                else
                    System.out.println("\nResquest is to fly from " + userDepartureCity + " to " + userDestination + ".\n"
                                             + "Sorry.  USAir does not fly from " + userDepartureCity + " to " + userDestination + ".");
                cityBuf.close();
                flightBuf.close();
            }
            catch (IOException ex)
            {
                System.out.println("Error: Invalid file(s).");
                break;
            }
        }
    }

    //---------------------------------------------------------End of Main Method--------------------------------------------------------

    public static City getCity(ArrayList<City> cities, int l, int r, String cityName)
    {
        if(r >= l)
        {
            int mid = l + (r - l)/2;
            if((cities.get(mid).getCityName()).compareTo(cityName) == 0)
                return cities.get(mid);
            if((cities.get(mid).getCityName()).compareTo(cityName) > 0)
                return getCity(cities, l, mid - 1, cityName);
            return getCity(cities, mid + 1, r, cityName);
        }
        return null;
    }

    public static int getTicketPrice(ArrayList<Flight> flights, City dest)
    {
        for(int i = 0; i < flights.size(); i++)
        {
            if(flights.get(i).getDestination() == dest)
                return flights.get(i).getTicketPrice();
        }
        return -1;
    }

    public static int getCityIndex(ArrayList<City> cities, int l, int r, String cityName)
    {
        if(r >= l)
        {
            int mid = l + (r - l)/2;
            if((cities.get(mid).getCityName()).compareTo(cityName) == 0)
                return mid;
            if((cities.get(mid).getCityName()).compareTo(cityName) > 0)
                return getCityIndex(cities, l, mid - 1, cityName);
            return getCityIndex(cities, mid + 1, r, cityName);
        }
        return -1;
    }
}
