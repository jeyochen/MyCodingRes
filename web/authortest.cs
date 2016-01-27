protected void Page_Load(object sender, EventArgs e)
{
	Session["username"] = "TestUser";
	string strAuthor = "1000,2000,30001,,,";

	List<string> authList = new List<string>(strAuthor.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries));

	if (VerifyAuthor.AuthorCheck(authList, "30001"))
	{
		;//Response.Redirect("http://www.126.com");
	}
	else
	{
		;//Response.Redirect("http://www.baidu.com");
	}

	if (ViewState["t1"] == null)
	{
		ViewState["t1"] = 0.00;
	}

	if (ViewState["t1"] == null)
	{
		ViewState["t2"] = DateTime.Now.Subtract(DateTime.Parse("1970-1-1")).TotalSeconds;
	}
}

public class VerifyAuthor
{
	public static bool AuthorCheck(List<string> authorlist, string author)
	{
		if (authorlist.Contains(author))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	public static void ArrListTest(ArrayList list)
	{
		;
	}

	public static bool JudgeAuthor()
	{
		return false;
	}
}