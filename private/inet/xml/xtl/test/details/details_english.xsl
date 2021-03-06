<?xml:namespace ns="uri:xsl" prefix="xsl"?>

<xsl:document>

	<HTML>
		<HEAD>
			<TITLE>Microsoft Investor: Quote Detail</TITLE>
			<LINK REL="STYLESHEET" HREF="details.css" TYPE="text/css"/>
		</HEAD>

		<BODY TOPMARGIN="0" LEFTMARGIN="0" BGCOLOR="white" TEXT="black" LINK="#3366cc" ALINK="#3366cc" VLINK="#663366">

			<A NAME="TOP"/>

			<TABLE WIDTH="100%" HEIGHT="64" BORDER="0" CELLSPACING="0" CELLPADDING="0" BGCOLOR="Black">
				<TR VALIGN="TOP">
					<TD>
						<A HREF="/home.asp" TARGET="_top">
							<IMG SRC="img/invlogo.gif" VSPACE="0" HSPACE="0" WIDTH="144" HEIGHT="48" ALIGN="LEFT" ALT="Microsoft Investor" BORDER="0"/>
						</A>
					</TD>

					<TD ALIGN="RIGHT" VALIGN="TOP">
						<A HREF="http://ads.msn.com/ads/redirect.dll/CID=0001baa20a72995b00000000/AREA=INVGEN?image=http://ads.msn.com/ads/INVGEN/P1CP0009_LG.GIF" target="_top">
							<IMG SRC="http://ads.msn.com/ads/INVGEN/P1CP0009_LG.GIF" ALT="Click here for Microsoft CarPoint. Your complete source for car buying information.." BORDER="0"/>
						</A>
					</TD>
				</TR>
			</TABLE>


			<TABLE HEIGHT="22" WIDTH="100%" CELLPADDING="0" CELLSPACING="0" BORDER="0" BGCOLOR="Black">
			<TR>
				<TD WIDTH="128"></TD>

				<TD BGCOLOR="Black">
					<A HREF="http://www.investor.com/home.asp" TARGET="_top" CLASS="navtop">Home</A>
				</TD>

				<TD>
					<A HREF="http://www.investor.com/controls/setup.asp?control=portfolio" TARGET="_top" CLASS="navtop">Portfolio</A>
				</TD>

				<TD>
					<A HREF="http://www.investor.com/prospect/articles/summary.asp" TARGET="_top" CLASS="navtop">Markets</A>
				</TD>

				<TD>
					<A HREF="http://www.investor.com/prospect/index/newtoday.asp" TARGET="_top" CLASS="navtop">Insight</A>
				</TD>

				<TD BGCOLOR="Black">
					<A HREF="http://www.investor.com/quotes/quotes.asp" TARGET="_top" CLASS="navtophighlight">Stocks</A>
				</TD>

				<TD>
					<A HREF="http://www.investor.com/quotes/quotes.asp?Funds=1" TARGET="_top" CLASS="navtop">Funds</A>
				</TD>

				<TD>
					<A HREF="http://www.investor.com/invsub/finder/screens.asp" TARGET="_top" CLASS="navtop">Finder</A>
				</TD>

				<TD>
					<A HREF="http://www.investor.com/trading/trade.asp" TARGET="_top" CLASS="navtop">Trading</A>
				</TD>

				<TD>
					<A HREF="http://www.investor.com/help/hcontent.asp" TARGET="_top" CLASS="navtop">Help</A>
				</TD>
				
				<TD>
					<FORM ACTION="" NAME="SNAVFORM">
					<TD>
						<SELECT SIZE="1" class="shared" NAME="quickbar" ONCHANGE="QbDcTEST(this);" language="JavaScript">
							<OPTION VALUE="http://network.msn.com/home/default.asp">- The Microsoft Network -</OPTION>
							<OPTION VALUE="http://network.msn.com/hotmail/hotmail.asp">Hotmail</OPTION>
							<OPTION VALUE="http://network.msn.com/hmc/hmc.asp">Microsoft Internet Start</OPTION>
							<OPTION VALUE="http://www.msn.com/xnav/default.asp">MSN.COM</OPTION>
							<OPTION VALUE="http://www.msnbc.com/xnav/default.asp">MSNBC News</OPTION>
							<OPTION VALUE="http://investor.msn.com/external/imgnav/imgnav.asp">Microsoft Investor</OPTION>
							<OPTION VALUE="http://expedia.msn.com/snav/default.asp">Expedia.com Travel</OPTION>
							<OPTION VALUE="http://www.zone.com/img/sharedNav.asp">Internet Gaming Zone</OPTION>
							<OPTION VALUE="http://network.msn.com/carpoint/carpoint.asp">Microsoft CarPoint</OPTION>
							<OPTION VALUE="http://expediamaps.com/snavredir/default.asp">Expedia Maps</OPTION>
							<OPTION VALUE="http://network.msn.com/sidewalk/sidewalk.asp">Sidewalk City Guides</OPTION>
							<OPTION VALUE="http://encarta.msn.com/teleport/fromSnav/gateway.asp">Microsoft Encarta</OPTION>
							<OPTION VALUE="http://network.msn.com/plaza/plaza.asp">Microsoft Plaza</OPTION>
							<OPTION VALUE="http://www.slate.com/code/sredir/comnav.asp">Slate</OPTION>
							<OPTION VALUE="http://network.msn.com/home/default.asp#more">More Microsoft sites...</OPTION>
							<OPTION VALUE="http://www.microsoft.com/">Microsoft.com</OPTION>
						</SELECT>

						<SCRIPT language="JavaScript">
                            <xsl:comment>
							function QbDcTEST(s)
							{	
								var d = s.options[s.selectedIndex].value;
								window.top.location.href = d;
								s.selectedIndex=0;
							}
                            </xsl:comment>
						</SCRIPT>
					</TD>
					</FORM>
				</TD>
			</TR>
			</TABLE>


			<TABLE HEIGHT="100%" WIDTH="100%" BORDER="0" CELLSPACING="0" CELLPADDING="0">
			<TR VALIGN="TOP">
				<TD WIDTH="128" BGCOLOR="Black">
					<TABLE WIDTH="128" BORDER="0" CELLPADDING="2" CELLSPACING="0">
					
					<TR BGCOLOR="#CC9900">
						<TD COLSPAN="2" HEIGHT="39">
							<DIV STYLE="color:White; margin-left: 8px; font-size:13pt; line-height: 20px;">
								<B>Stock</B>
							</DIV>
							<DIV STYLE="color: White; margin-left: 42px; font-size:13pt; line-height: 19px;">Research</DIV>
						</TD>
					</TR>

					<TR>
						<TD WIDTH="16"></TD>
						<TD WIDTH="112" BGCOLOR="White">
							<A HREF="http://www.investor.com/controls/setup.asp?symbol=msft" TARGET="_top" CLASS="navlefthighlight1">Quote Detail</A>
						</TD>
					</TR>

					<TR>
						<TD></TD>
						<TD>
							<A HREF="http://www.investor.com/controls/setup.asp?symbol=msft" TARGET="_top" CLASS="navleft1">Historical Charts</A>
						</TD>
					</TR>

					<TR>
						<TD></TD>
						<TD>
							<A HREF="http://www.investor.com/news/pressrel.asp?Symbol=msft" TARGET="_top" CLASS="navleft1">Company News</A>
						</TD>
					</TR>

					<TR>
						<TD></TD>
						<TD>
							<A HREF="http://www.investor.com/research/snapshot.asp?Symbol=msft" TARGET="_top" CLASS="navleft1">Company Facts</A>
						</TD>
					</TR>

					<TR>
						<TD></TD>
						<TD>
							<A HREF="http://www.investor.com/sec/mda.asp?Symbol=msft" TARGET="_top" CLASS="navleft1">SEC Filings</A>
						</TD>
					</TR>

					<TR>
						<TD>
							<IMG SRC="img/star1.gif" WIDTH="10" HEIGHT="10" VSPACE="0" BORDER="0"/>
						</TD>
						<TD>
							<A HREF="http://www.investor.com/invsub/advisor/advisor.asp?Symbol=msft" TARGET="_top" CLASS="navleft1">Advisor FYI</A>
						</TD>
					</TR>

					<TR>
						<TD>
							<IMG SRC="img/star1.gif" WIDTH="10" HEIGHT="10" VSPACE="0" BORDER="0"/>
						</TD>
						<TD>
							<A HREF="http://www.investor.com/invsub/business/overview.asp?Symbol=msft" TARGET="_top" CLASS="navleft1">Business Profile</A>
						</TD>
					</TR>

					<TR>
						<TD>
							<IMG SRC="img/star1.gif" WIDTH="10" HEIGHT="10" VSPACE="0" BORDER="0"/>
						</TD>
						<TD>
							<A HREF="http://www.investor.com/invsub/analyst/earnest.asp?Symbol=msft" TARGET="_top" CLASS="navleft1">Analyst Info</A>
						</TD>
					</TR>

					<TR>
						<TD>
							<IMG SRC="img/star1.gif" WIDTH="10" HEIGHT="10" VSPACE="0" BORDER="0"/>
						</TD>
						<TD>
							<A HREF="http://www.investor.com/invsub/results/hilite.asp?Symbol=msft" TARGET="_top" CLASS="navleft1">Financial Results</A>
						</TD>
					</TR>

					<TR>
						<TD>
							<IMG SRC="img/star1.gif" WIDTH="10" HEIGHT="10" VSPACE="0" BORDER="0"/>
						</TD>
						<TD>
							<A HREF="http://www.investor.com/invsub/insider/trans.asp?Symbol=msft" TARGET="_top" CLASS="navleft1">Insider Trading</A>
						</TD>
					</TR>
					</TABLE>
				</TD>
				
				<TD WIDTH="100%">

				<TABLE WIDTH="100%" BORDER="0" CELLSPACING="0" CELLPADDING="15">
				<TR>
                                     <TD>


					<TABLE BORDER="0" CELLPADDING="0" CELLSPACING="0">
				
						<FORM ACTION="details.xsp" METHOD="GET">
							<TR NOWRAP="true" VALIGN="MIDDLE">
								<TD>Enter Symbol: <INPUT TYPE="TEXT" SIZE="12"  NAME="symbol" VALUE=""/></TD>
								<TD WIDTH="15"></TD>
								<TD> <INPUT TYPE="SUBMIT" SIZE="50"  VALUE="Enter"/></TD>
								<TD WIDTH="15"></TD>
							</TR>
						</FORM>
					</TABLE>
			
					<IMG SRC="img/1x1grn.gif" VSPACE="3" HSPACE="0" BORDER="0" HEIGHT="1" ALIGN="BOTTOM" WIDTH="100%"/>
				
					<TABLE BORDER="0" WIDTH="100%">
						<TR>
							<TD VALIGN="bottom">

								<A HREF="/quotes/fullquot.asp?Symbol=" class="fyistyle">fyi</A>

								<SPAN CLASS="smallprompt">
									Quotes delayed 15 minutes. Last Updated: <xsl:get-value for="xml/time"/>
								</SPAN>
							</TD>

							<TD ALIGN="right" VALIGN="bottom"></TD>
						</TR>
					</TABLE>
	
	
					<TABLE BORDER="1" CELLPADDING="1" CELLSPACING="0" RULES="rows" FRAME="below" BORDERCOLOR="#cccc99" BORDERCOLORDARK="white">
						<TR BGCOLOR="#336666">
							<TH ALIGN="left" COLSPAN="7" HEIGHT="18" NOWRAP="true">
								<SPAN CLASS="smallfontreverse">
						   			<B><xsl:get-value for="xml/row/name"/></B>
								</SPAN>
							</TH>
						</TR>

						<TR>
							<TD ALIGN="right"></TD>
							<TD>Last</TD>
							<TD ALIGN="right" NOWRAP="true">
								<B> <xsl:get-value for="xml/row/last"/> </B>
							</TD>
							<TD WIDTH="20" ROWSPAN="6"></TD>
							<TD ALIGN="right"></TD>
							<TD>Open</TD>
							<TD ALIGN="right" NOWRAP="true">
								<xsl:get-value for="xml/row/open"/>
							</TD>
						</TR>


						<TR>
							<TD ALIGN="right"></TD>
							<TD>Change</TD>
							<TD ALIGN="right" NOWRAP="true">
								<B> <xsl:get-value for="xml/row/change"/> </B>
							</TD>
							
							<TD ALIGN="right"></TD>
							<TD>Previous Close</TD>
							<TD ALIGN="right" NOWRAP="true">
								<xsl:get-value for="xml/row/previousclose"/>
							</TD>
						</TR>

						<TR>
							<TD ALIGN="right"></TD>
							<TD>% Change </TD>
							<TD ALIGN="right" NOWRAP="true">
								<xsl:get-value for="xml/row/change"/>
							</TD>
							<TD ALIGN="right"></TD>
							<TD>Bid</TD>
							<TD ALIGN="right" NOWRAP="true">
								<xsl:get-value for="xml/row/bid"/>
							</TD>
						</TR>

						<TR>
							<TD ALIGN="right"></TD>
							<TD>Volume</TD>
							<TD ALIGN="right" NOWRAP="true">
								<xsl:get-value for="xml/row/volume"/>
							</TD>
							<TD ALIGN="right"></TD>
							<TD>Ask</TD>
							<TD ALIGN="right" NOWRAP="true">
								<xsl:get-value for="xml/row/ask"/>
							</TD>
						</TR>

						<TR>
							<TD ALIGN="right"></TD>
							<TD>Day's High</TD>
							<TD ALIGN="right" NOWRAP="true">
								<xsl:get-value for="xml/row/dayhigh"/>
							</TD>
							<TD ALIGN="right"></TD>
							<TD>52 Week High</TD>
							<TD ALIGN="right" NOWRAP="true">
								<xsl:get-value for="xml/row/yearhigh"/>
							</TD>
						</TR>

						<TR>
							<TD ALIGN="right"></TD>
							<TD>Day's Low</TD>
							<TD ALIGN="right" NOWRAP="true">
								<xsl:get-value for="xml/row/daylow"/>
							</TD>
							<TD ALIGN="right"></TD>
							<TD>52 Week Low</TD>
							<TD ALIGN="right" NOWRAP="true">
								<xsl:get-value for="xml/row/yearlow"/>
							</TD>
						</TR>

						<TR>
							<TD COLSPAN="7">
								<BR/><SPAN CLASS="smallfont">Fundamental Data</SPAN>
							</TD>
						</TR>

						<TR>
							<TD ALIGN="right"></TD>
							<TD>P/E</TD>
							<TD ALIGN="right" NOWRAP="true">
								<xsl:get-value for="xml/row/pe"/>
							</TD>
							<TD WIDTH="20" ROWSPAN="4"></TD>
							<TD ALIGN="right"></TD>
							<TD>Market Cap.</TD>
							<TD ALIGN="right" NOWRAP="true">
								<xsl:get-value for="xml/row/marketcap"/>
							</TD>
						</TR>

						<TR>
							<TD ALIGN="right"></TD>
							<TD>Earnings/Share</TD>
							<TD ALIGN="right" NOWRAP="true">
								<xsl:get-value for="xml/row/earnings"/>
							</TD>
							<TD ALIGN="right"></TD>
							<TD># Shares Out.</TD>
							<TD ALIGN="right" NOWRAP="true">					
								<xsl:get-value for="xml/row/sharesout"/>
							</TD>
						</TR>

						<TR>
							<TD ALIGN="right"></TD>
							<TD>Dividend/Share</TD>
							<TD ALIGN="right" NOWRAP="true">
								<xsl:get-value for="xml/row/dividend"/>
							</TD>
							<TD ALIGN="right"></TD>
							<TD>Exchange</TD>
							<TD ALIGN="right" NOWRAP="true">
								<xsl:get-value for="xml/row/exchange"/>
							</TD>
						</TR>

						<TR>
							<TD ALIGN="right"></TD>
							<TD>Yield</TD>
							<TD ALIGN="right" NOWRAP="true">
								<xsl:get-value for="xml/row/yield"/>
							</TD>
							<TD></TD>
							<TD COLSPAN="2">
								<SPAN CLASS="smallprompt">
									<A HREF="/controls/setup.asp">Intraday Chart</A>
								</SPAN>
							</TD>
						</TR>
					</TABLE>
				
					<BR/>

					<SPAN CLASS="smallprompt">
						<A HREF="/common/disclaim.asp" TARGET="_top">Terms of Use, and 
							Privacy Policy. 1996-98 Microsoft Corporation and/or its suppliers. All 
							rights reserved.
						</A>
					
						<BR/>
						<BR CLEAR="all"/>		
					</SPAN>


					<A HREF="http://www.spcomstock.com/" TARGET="_top">
						<IMG SRC="img/splgo.gif" HSPACE="2" VSPACE="0" BORDER="0" ALIGN="LEFT" WIDTH="87" HEIGHT="17"/>
					</A>

					Quotes supplied by Standard &amp; Poor's ComStock, Inc.<BR/><BR CLEAR="ALL"/>

					<A HREF="http://www.investor.com/common/attrib/mg.asp">
						<IMG SRC="img/mglgo.gif" HSPACE="2" VSPACE="0" BORDER="0" ALIGN="LEFT" WIDTH="87" HEIGHT="21"/>
					</A>

					Stock data provided by <A HREF="http://www.investor.com/common/attrib/mg.asp">Media General Financial Services</A>.

				    </TD>
				</TR>
				</TABLE>
				</TD>
			</TR>
			</TABLE>
		</BODY>

	</HTML>

</xsl:document>