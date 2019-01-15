public final class rsa_private_key
{
    public FF p,q,dp,dq,c;
	
	public rsa_private_key(int n)
	{
		p=new FF(n);
		q=new FF(n);
		dp=new FF(n);
		dq=new FF(n);
		c=new FF(n);
	}
}