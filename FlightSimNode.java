//Linked list utilizes the node class
public class FlightSimNode
{
    private City city;
    private Node next;

    public Node(City newCity)
    {
        city = newCity;
        next = null;
    }

    public Node (City newCity, Node nextNode)
    {
        city = newCity;
        next = nextNode;
    }

    public City getCity()
    {
        return city;
    }

    public Node getNext()
    {
        return next;
    }

    public void setNext(Node nextNode)
    {
        next = nextNode;
    }
}
