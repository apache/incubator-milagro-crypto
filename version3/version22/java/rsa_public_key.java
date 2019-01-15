public final class rsa_public_key
{
    public int e;
    public FF n;

	public rsa_public_key(int m)
	{
		e=0;
		n=new FF(m);
	}
}
