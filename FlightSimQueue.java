public class FlightSimQueue
{
    private Node b;

    public Queue()
    {
        b = null;
    }

    public boolean isEmpty()
    {
        return(b == null);
    }

    public void enqueue(City newCity)
    {
        Node newNode = new Node(newCity);

        if(b == null)
        {
            newNode.setNext(newNode);
            b = newNode;
        }
        else
        {
            newNode.setNext(b.getNext());
            b.setNext(newNode);
            b = newNode;
        }
    }

    public City dequeue() throws RuntimeException
    {
        if(!isEmpty())
        {
            Node x = b.getNext();
            if(x == b)
                b = null;
            else
            {
                b.setNext(x.getNext());
                x.setNext(null);
            }

            return x.getCity();
        }
        else
            throw new RuntimeException("RTE");
    }
}
