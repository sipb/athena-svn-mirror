<?xml version='1.0'?>
<!DOCTYPE xsl:stylesheet [
<!ENTITY sep '" "'>
<!ENTITY comma '", "'>
]>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version='1.0'>

<!-- copyright (c) 2001 Sun Microsystems, Inc. -->

<xsl:output method="xml" indent="yes"/>


<xsl:key name="primary"
         match="indexterm[@id | @zone | see]" 
         use="normalize-space(primary)"/>

<xsl:key name="secondary"
         match="indexterm[@id | @zone | see]" 
         use="normalize-space(concat(primary, &sep;, secondary))"/>

<xsl:key name="tertiary"
         match="indexterm[@id | @zone | see]" 
         use="normalize-space(concat(primary, &sep;, secondary, &sep;, tertiary))"/>

<xsl:key name="primary-section"
         match="indexterm[@id | @zone and not(secondary) and not(see)]"
         use="@id|@zone"/>

<xsl:key name="secondary-section"
         match="indexterm[@id | @zone and not(tertiary) and not(see)]"
         use="normalize-space(concat(primary, &sep;, secondary))"/>

<xsl:key name="tertiary-section"
	 match="indexterm[@id | @zone and not(see)]"
	 use="normalize-space(concat(primary, &sep;, secondary, &sep;, tertiary))"/>

<xsl:key name="primary-term"
         match="indexterm[@id | @zone and not(secondary) and not(see)]"
         use="normalize-space(primary)"/>

<xsl:key name="see-also"
	 match="indexterm[seealso]"
	 use="normalize-space(concat(primary, &sep;, secondary, &sep;, tertiary, &sep;, seealso))"/>

<xsl:key name="see"
	 match="indexterm[see]"
	 use="normalize-space(concat(primary, &sep;, secondary, &sep;, tertiary, &sep;, see))"/>

<xsl:key name="see-reference"
	 match="indexterm"
	 use="normalize-space(translate(see|seealso, '&quot;,', &sep;))"/>


<xsl:template match="/">
    <indexdoc>
        <xsl:apply-templates select="//indexterm[@id | @zone | see and count(.|key('primary', primary)[1])=1]" mode="index-primary">
           <xsl:sort select="primary"/>
        </xsl:apply-templates>
    </indexdoc>
</xsl:template>


<xsl:template match="indexterm" mode="index-primary">
   <indexitem>
      <xsl:variable name="key" select="normalize-space(primary)"/>
      <xsl:variable name="refs" select="key('primary', $key)"/>
      <title><xsl:value-of select="primary"/></title>
      <xsl:for-each select="$refs[key('primary-section', @id|@zone)]">
         <link><xsl:attribute name="linkid">
                   <xsl:value-of select="@id|@zone"/>
	    </xsl:attribute>
            <xsl:if test="key('see-reference', $key)">
                <xsl:attribute name="indexid">
                   <xsl:value-of select="generate-id(key('primary-term', $key)[1])"/>
                </xsl:attribute> 
            </xsl:if>
          </link>
       </xsl:for-each>
       <xsl:if test="$refs/secondary or $refs[not(secondary)]/*[self::see or self::seealso]">
            <xsl:apply-templates select="$refs[generate-id() = generate-id(key('see', normalize-space(concat(primary, &sep;, &sep;, &sep;, see)))[1])]"
                       mode="index-see">
                <xsl:sort select="see"/>
            </xsl:apply-templates>
            <xsl:apply-templates select="$refs[generate-id() = generate-id(key('see-also', normalize-space(concat(primary, &sep;, &sep;, &sep;, seealso)))[1])]"
                       mode="index-seealso">
               <xsl:sort select="seealso"/>
            </xsl:apply-templates>
            <xsl:apply-templates select="$refs[secondary and count(.|key('secondary', normalize-space(concat($key, &sep;, secondary)))[1]) = 1]"
                             mode="index-secondary">
               <xsl:sort select="secondary"/>
	     </xsl:apply-templates>
         </xsl:if>    
     </indexitem>
</xsl:template>

<xsl:template match="indexterm" mode="index-secondary">
   <indexitem>
      <xsl:variable name="key" select="normalize-space(concat(primary, &sep;, secondary))"/>
      <xsl:variable name="refs" select="key('secondary', $key)"/>
      <title><xsl:value-of select="secondary"/></title>
      <xsl:for-each select="$refs[key('secondary-section', $key)]">
         <link><xsl:attribute name="linkid">
                   <xsl:value-of select="@id|@zone"/>
	    </xsl:attribute>
            <xsl:if test="key('see-reference', $key)">
                <xsl:attribute name="indexid">
                   <xsl:value-of select="generate-id(key('secondary-section', $key)[1])"/>
                </xsl:attribute> 
            </xsl:if>
          </link>
       </xsl:for-each>
      <xsl:if test="$refs/tertiary or $refs[not(tertiary)]/*[self::see or self::seealso]">
         <xsl:apply-templates select="$refs[generate-id() = generate-id(key('see', normalize-space(concat(primary, &sep;, secondary, &sep;, &sep;, see)))[1])]"
                       mode="index-see">
            <xsl:sort select="see"/>
         </xsl:apply-templates>
         <xsl:apply-templates select="$refs[generate-id() = generate-id(key('see-also', normalize-space(concat(primary, &sep;, secondary, &sep;, &sep;, seealso)))[1])]"
                       mode="index-seealso">
            <xsl:sort select="seealso"/>
         </xsl:apply-templates>
         <xsl:apply-templates select="$refs[tertiary and count(.|key('tertiary',
normalize-space(concat($key, &sep;, tertiary)))[1]) = 1]"
                             mode="index-tertiary">
            <xsl:sort select="tertiary"/>
         </xsl:apply-templates>
     </xsl:if>    
   </indexitem>
</xsl:template>

<xsl:template match="indexterm" mode="index-tertiary">
   <indexitem>
      <xsl:variable name="key" select="normalize-space(concat(primary, &sep;, secondary, &sep;, tertiary))"/>
      <xsl:variable name="refs" select="key('tertiary', $key)"/>
      <title><xsl:value-of select="tertiary"/></title>
      <xsl:for-each select="$refs[key('tertiary-section', $key)]">
         <link><xsl:attribute name="linkid">
                   <xsl:value-of select="@id|@zone"/>
	    </xsl:attribute>
            <xsl:if test="key('see-reference', $key)">
                <xsl:attribute name="indexid">
                   <xsl:value-of select="generate-id(key('tertiary-section', $key)[1])"/>
                </xsl:attribute> 
            </xsl:if>
          </link>
       </xsl:for-each>
      <xsl:variable name="reference" select="$refs/seealso | $refs/see"/>
      <xsl:if test="$reference">
        <xsl:apply-templates select="$refs[generate-id() = generate-id(key('see', normalize-space(concat(primary, &sep;, secondary, &sep;, tertiary, &sep;, see)))[1])]"
                      mode="index-see">
           <xsl:sort select="see"/>
        </xsl:apply-templates>
        <xsl:apply-templates select="$refs[generate-id() = generate-id(key('see-also', normalize-space(concat(primary, &sep;, secondary, &sep;, tertiary, &sep;, seealso)))[1])]"
                       mode="index-seealso">
             <xsl:sort select="seealso"/>
         </xsl:apply-templates>
      </xsl:if>
   </indexitem>
</xsl:template>

<xsl:template match="indexterm" mode="index-see">
   <xsl:variable name="key" select="normalize-space(translate(see, '&quot;', &sep;))"/>
   <xsl:choose>
      <xsl:when test="not(contains($key, ','))">
         <see><xsl:attribute name="indexid">
	    <xsl:value-of select="generate-id(key('primary-term', $key)[1])"/>
	    </xsl:attribute>
	    <xsl:value-of select="$key"/>
         </see>
      </xsl:when>
      <xsl:otherwise>
         <xsl:variable name="first-term" select="normalize-space(substring-before($key, ','))"/>
         <xsl:variable name="rest" select="normalize-space(substring-after($key, ','))"/>
         <xsl:choose>
            <xsl:when test="substring-after($rest, ',')">
               <xsl:variable name="second-term" select="normalize-space(substring-before($rest, ','))"/>
               <xsl:variable name="third-term" select="normalize-space(substring-after($rest, ','))"/>
               <see><xsl:attribute name="indexid">
		    <xsl:value-of select="generate-id(key('tertiary-section', concat($first-term, &sep;, $second-term, &sep;, $third-term))[1])"/>
		    </xsl:attribute>
		    <xsl:value-of select="concat($first-term, &comma;, $second-term, &comma;, $third-term)"/>
		</see>
            </xsl:when>
            <xsl:otherwise>
               <see><xsl:attribute name="indexid">
	            <xsl:value-of select="generate-id(key('secondary-section', concat($first-term, &sep;, $rest))[1])"/>
	            </xsl:attribute>
	            <xsl:value-of select="concat($first-term, &comma;, $rest)"/>
	       </see>
             </xsl:otherwise>
          </xsl:choose>
      </xsl:otherwise>
    </xsl:choose>
</xsl:template>

<xsl:template match="indexterm" mode="index-seealso">
   <xsl:variable name="key" select="normalize-space(translate(seealso, '&quot;', &sep;))"/>
   <xsl:choose>
	<xsl:when test="not(contains($key, ','))">
	 <seealso><xsl:attribute name="indexid">
            <xsl:value-of select="generate-id(key('primary-term', $key)[1])"/>
            </xsl:attribute>
            <xsl:value-of select="$key"/>
	 </seealso>
	</xsl:when>
	<xsl:otherwise>
	 <xsl:variable name="first-term" select="normalize-space(substring-before($key, ','))"/>
	 <xsl:variable name="rest" select="normalize-space(substring-after($key, ','))"/>
	 <xsl:choose>
            <xsl:when test="substring-after($rest, ',')">
		<xsl:variable name="second-term" select="normalize-space(substring-before($rest, ','))"/>
                <xsl:variable name="third-term" select="normalize-space(substring-after($rest, ','))"/>
		<seealso><xsl:attribute name="indexid">
                    <xsl:value-of select="generate-id(key('tertiary-section', concat($first-term, &sep;, $second-term, &sep;, $third-term))[1])"/>
                    </xsl:attribute>
                    <xsl:value-of select="concat($first-term, &comma;, $second-term, &comma;, $third-term)"/>
		</seealso>
            </xsl:when>
            <xsl:otherwise>
		<seealso><xsl:attribute name="indexid">
                    <xsl:value-of select="generate-id(key('secondary-section', concat($first-term, &sep;, $rest))[1])"/>
               	    </xsl:attribute>
                    <xsl:value-of select="concat($first-term, &comma;, $rest)"/>
		</seealso>
             </xsl:otherwise>
	  </xsl:choose>
	</xsl:otherwise>
    </xsl:choose>
</xsl:template>



</xsl:stylesheet>
