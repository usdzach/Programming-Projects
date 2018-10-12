/* Author: Zach Fukuhara <zfukuhara@sandiego.edu>
 *
 * Description:  Helper class for FlightBookingSimulator.java which
 * represents a flight to a given destination city for a given ticket price
 */

public class Flight
{
    private City destination;
    private int ticketPrice;

    public Flight(City newDest, int newPrice)
    {
        destination = newDest;
        ticketPrice = newPrice;
    }

    public City getDestination()
    {
        return destination;
    }

    public int getTicketPrice()
    {
        return ticketPrice;
    }
}
