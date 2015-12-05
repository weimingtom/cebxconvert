package cebx;

import java.io.IOException;
import java.io.InputStream;
import java.util.Scanner;

public class CebxConverter {
	private static final String EXE_PATH = 
		//"D:\\Users\\administrator\\documents\\visual studio 2010\\Projects\\cebxconvert\\Release\\cebxconvert.exe";
		"D:\\Program Files (x86)\\Founder\\Apabi Maker 5.0\\Bin\\cebxconvert.exe";
	private StringBuffer outputBuffer = new StringBuffer();

	public String getOutput() {
		return outputBuffer.toString();
	}

	public int convert(String input, String output) {
		outputBuffer.setLength(0);
		final String[] commands = {
				EXE_PATH,
				//"-v", 
				input, output};
		final ProcessBuilder pb = new ProcessBuilder(commands);
		Process process = null;
		InputStream in = null;
		try {
			process = pb.redirectErrorStream(true).start();
			in = process.getInputStream();
			Scanner scanner = new Scanner(in, "gbk"); //encoding
			while (scanner.hasNextLine()) {
				String line = scanner.nextLine();
				outputBuffer.append(line);
				outputBuffer.append("\n");
			}
			if (scanner.ioException() != null) {
				return -1;
			}
			return process.exitValue();
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			if (in != null) {
				try {
					in.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		return -2;
	}
	
	public static final void main(String[] args) {
		System.out.println("======cebx convert start...");
		
		CebxConverter c = new CebxConverter();
		int ret = c.convert("D:\\a.ceb", "D:\\b.cebx");
		
		System.out.println(c.getOutput());
		
		System.out.println("======return value : " + ret);
		System.out.println("======cebx convert end...");
	}
}
