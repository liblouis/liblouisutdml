package org.liblouis;

import java.io.IOException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.io.InputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.List;
import java.util.ArrayList;

import javax.xml.parsers.*;
import javax.xml.xpath.*;

import org.w3c.dom.*;
import org.xml.sax.SAXException;

import org.testng.annotations.AfterMethod;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;
import static org.testng.Assert.fail;
import static org.testng.Assert.assertEquals;

import org.liblouis.LibLouisUTDML;
import org.liblouis.LogLevel;

public class SimpleTest {
  LibLouisUTDML lbu;
  @BeforeClass
  public void LoadLibLouisUTDML() throws Exception {
    LibLouisUTDML.loadLibrary(System.getProperty("liblouis.dir"), System.getProperty("liblouis.ext"));
    lbu = LibLouisUTDML.getInstance();
    lbu.setDataPath(new File("testdata").getAbsolutePath());
    lbu.setLogLevel(LogLevel.FATAL);
  }
  @AfterMethod
  public void freeLibLouisUTDML() {
    lbu.free();
  }
  private String getTextFromNode(XPathExpression expr, Node searchNode) throws XPathExpressionException {
    NodeList resultList = (NodeList)expr.evaluate(searchNode, XPathConstants.NODESET);
    if (resultList.getLength() == 0) {
      return null;
    }
    return resultList.item(0).getNodeValue();
  }
  private int getIntFromNode(XPathExpression expr, Node searchNode, int defaultValue) throws XPathExpressionException {
    int result = defaultValue;
    String resultStr = "";
    try {
      if ((resultStr = getTextFromNode(expr, searchNode)) != null) {
        result = Integer.parseInt(resultStr);
      }
    } catch (NumberFormatException e) {
      result = defaultValue;
    }
    return result;
  }
  private void closeStream(InputStream s) {
    try {
      s.close();
    } catch (IOException e) {
      // No action needed
    }
  }
  private String readFileToString(String filename, String encoding) {
    FileInputStream in = null;
    BufferedReader br = null;
    StringBuilder builder = new StringBuilder();
    try {
      in = new FileInputStream(filename);
      br = new BufferedReader(new InputStreamReader(in, encoding));
      int read, n = 1024 * 1024;
      char[] buf = new char[n];
      while ((read = br.read(buf, 0, n)) != -1) {
        builder.append(buf, 0, read);
      }
    } catch (IOException e) {
      System.out.println("Problem reading data file");
    } finally {
      if (in != null) {
        closeStream(in);
      }
    }
    return builder.toString();
  }
  @DataProvider(name="translateFileTest")
  public Object[][] translateFileData() throws ParserConfigurationException, SAXException, XPathExpressionException, IOException {
    DocumentBuilderFactory domFactory = DocumentBuilderFactory.newInstance();
    domFactory.setNamespaceAware(true);
    DocumentBuilder builder = domFactory.newDocumentBuilder();
    Document doc = builder.parse(new File("testdata", "tests.xml").getAbsolutePath());
    XPathFactory factory = XPathFactory.newInstance();
    XPath xpath = factory.newXPath();
    XPathExpression testsExpr = xpath.compile("/tests/translateFileTest");
    XPathExpression configListExpr = xpath.compile("configList/text()");
    XPathExpression inFileExpr = xpath.compile("inFile/text()");
    XPathExpression expectedOutFileExpr = xpath.compile("expectedOutFile/text()");
    XPathExpression modeExpr = xpath.compile("mode/text()");
    XPathExpression logFileExpr = xpath.compile("logFile/text()");
    XPathExpression settingsExpr = xpath.compile("settings/text()");
    NodeList testNodes = (NodeList)testsExpr.evaluate(doc, XPathConstants.NODESET);
    List<Object[]> tests = new ArrayList<Object[]>();
    Object[] testCase;
    for (int i = 0; i < testNodes.getLength(); i++) {
      testCase = new Object[6];
      if ((testCase[0] = getTextFromNode(configListExpr, testNodes.item(i))) == null) {
        System.out.println("Test with no configList being skipped");
        continue;
      }
      if ((testCase[1] = getTextFromNode(inFileExpr, testNodes.item(i))) == null) {
        System.out.println("Test has no inFile so skipping");
        continue;
      }
      if ((testCase[2] = getTextFromNode(expectedOutFileExpr, testNodes.item(i))) == null) {
        System.out.println("Test has no expectedOutFile so skipping");
        continue;
      }
      testCase[3] = getTextFromNode(logFileExpr, testNodes.item(i));
      testCase[4] = getTextFromNode(settingsExpr, testNodes.item(i));
      testCase[5] = getIntFromNode(modeExpr, testNodes.item(i), 0);
      tests.add(testCase);
    }
    return tests.toArray(new Object[tests.size()][6]);
  }
  @Test(dataProvider="translateFileTest")
  public void testTranslateFile(String configList, String inFile, String expectedFile, String logFile, String settings, int mode) throws Exception {
    String absoluteLogFilePath = null;
    if (logFile != null)
    {
      absoluteLogFilePath = new File(logFile).getAbsolutePath();
    }
    // Perform the translation of the file
    if (!lbu.translateFile(configList, new File(inFile).getAbsolutePath(), new File("testdata", "actual.utd").getAbsolutePath(), absoluteLogFilePath, settings, mode)) {
      throw new Exception("LibLouisUTDML could not perform translation of the file " + inFile);
    }
    // Now compare the results
    BufferedReader actualReader = new BufferedReader(new FileReader(new File("testdata", "actual.utd")));
    BufferedReader expectedReader = new BufferedReader(new FileReader(new File(expectedFile)));
    String actualLine;
    String expectedLine;
    // Do not use the && shortcut as we always want to read both files
    while (((actualLine = actualReader.readLine()) != null) & ((expectedLine = expectedReader.readLine()) != null)) {
      if (!actualLine.equals(expectedLine)) {
        // Set one of the line strings to null so that fail will be detected
        actualLine = null;
        break;
      }
    }
    // Check for failure, only one of actualLine or expectedLine will be null
    if ((actualLine == null) ^ (expectedLine == null)) {
      fail("The actual file does not match the expected file");
    }
  }
  @Test(enabled=false)
  public void testBackTranslateFile() {
    fail("Not implemented yet");
  }
  @DataProvider(name="translateStringTest")
  public Object[][] translateStringData() throws IOException, ParserConfigurationException, SAXException, XPathExpressionException {
    DocumentBuilderFactory domFactory = DocumentBuilderFactory.newInstance();
    domFactory.setNamespaceAware(true);
    DocumentBuilder builder = domFactory.newDocumentBuilder();
    Document doc = builder.parse(new File("testdata", "tests.xml").getAbsolutePath());
    XPathFactory factory = XPathFactory.newInstance();
    XPath xpath = factory.newXPath();
    XPathExpression testsExpr = xpath.compile("/tests/translateStringTest");
    XPathExpression configListExpr = xpath.compile("configList/text()");
    XPathExpression inbufExpr = xpath.compile("inFile/text()");
    XPathExpression outbufExpr = xpath.compile("expectedOutFile/text()");
    XPathExpression logFileNameExpr = xpath.compile("logFileName/text()");
    XPathExpression settingsExpr = xpath.compile("settings/text()");
    XPathExpression modeExpr = xpath.compile("mode/text()");
    List<Object[]> tests = new ArrayList<Object[]>();
    Object[] testCase;
    NodeList testNodes = (NodeList)testsExpr.evaluate(doc, XPathConstants.NODESET);
    String tmpStr = null;
    for (int i = 0; i < testNodes.getLength(); i++) {
      testCase = new Object[7];
      if ((testCase[0] = getTextFromNode(configListExpr, testNodes.item(i))) == null) {
        System.out.println("No configList so skipping test");
        continue;
      }
      if ((tmpStr = getTextFromNode(inbufExpr, testNodes.item(i))) != null) {
        testCase[1] = readFileToString(tmpStr, "UTF8");
      } else {
        System.out.println("No inbuf defined");
        continue;
      }
      if ((tmpStr = getTextFromNode(outbufExpr, testNodes.item(i))) != null) {
        testCase[2] = readFileToString(tmpStr, "UTF8");
      } else {
        System.out.println("No outbuf defined");
        continue;
      }
      testCase[3] = ((String)testCase[2]).length() + 1;
      testCase[4] = getTextFromNode(logFileNameExpr, testNodes.item(i));
      testCase[5] = getTextFromNode(settingsExpr, testNodes.item(i));
      testCase[6] = getIntFromNode(modeExpr, testNodes.item(i), 0);
      tests.add(testCase);
    }
    return tests.toArray(new Object[tests.size()][7]);
  }
  @Test(dataProvider="translateStringTest")
  public void testTranslateString(String configList, String inbuf, String outbuf, int outlen, String logFilename, String settingsString, int mode) throws Exception {
    byte[] actualOutbuf = new byte[outlen*4];
    byte[] inbufBytes = java.util.Arrays.copyOf(inbuf.getBytes("UTF-8"), inbuf.length()+1);
    int[] outlenArray = new int[] { outlen };
    if (!lbu.translateString(configList, inbufBytes, actualOutbuf, outlenArray, logFilename, settingsString, mode)) {
      throw new Exception("LibLouisUTDML was unable to perform the translation");
    }
    String actualResult = new String(actualOutbuf, 0, outlenArray[0], "UTF-8");
    assertEquals(outbuf, actualResult);
  }
}
