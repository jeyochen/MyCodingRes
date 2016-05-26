import java.util.Scanner;
import com.sun.jna.Library;
import com.sun.jna.Native;
public class TestSo {
    public interface LgetLib extends Library {
        LgetLib INSTANCE = (LgetLib)Native.loadLibrary("XZ_F10_API_LINUX",LgetLib.class);
        
        boolean SetSerialParam(int iPort, int iBaud);
        boolean SetWorkKey(String szData);
        String Encrypt(int iType, String szData);
    }

    public boolean SetSerialParam(int iPort, int iBaud){
        return LgetLib.INSTANCE.SetSerialParam(iPort, iBaud);
    }
    

    public boolean SetWorkKey(String szData){
        return LgetLib.INSTANCE.SetWorkKey(szData);
    }

    public String Encrypt(int iType, String szData){
        return LgetLib.INSTANCE.Encrypt(iType, szData);
    }
	
	private void ShowMenu(){
		System.out.println("               ************************************");
		System.out.println("               *      2.MAC加密                    ");
		System.out.println("               *      3.注入密钥                   ");
		System.out.println("               *      9.退出程序                   ");
		System.out.println("               ************************************");
	}

    public static void main(String[] args) {
		if (0 >= args.length){
			System.out.println("请输入串口端口号!");
			return;
		}
		
        TestSo ts = new TestSo();
        int port = Integer.parseInt(args[0]);
		if (0 > port || port > 128){
			System.out.println("串口号输入错误：" + port);
			return;
		}
        System.out.println("port = "+port);
        // 设置串口
        if (false == ts.SetSerialParam(1, 9600)){
            System.out.println("设置串口失败");
        }
		
		ts.ShowMenu();
        Scanner input = new Scanner(System.in);
        while(true){
			System.out.println("请输入命令:");
			int cmd = input.nextInt();
			System.out.println("cmd= "+cmd);
			if(2 == cmd){
				String strData = "990123456789";
				String strEnc = ts.Encrypt(1, strData);
				System.out.println("加密结果：" + strEnc);
			}else if (3 == cmd){
				String strData = "F15FD36B806E943564D874057E745256DF5CACBA37E80712BFD1A389000000000000000046BE16C0";
			    ts.SetWorkKey(strData);
			}else if (9 == cmd){
				break;
			}
			//ts.ShowMenu();
			break;
		}
    }
}
