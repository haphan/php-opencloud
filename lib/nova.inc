<?php
// (c)2012 Rackspace Hosting
// See COPYING for licensing information

namespace OpenCloud;

require_once('service.inc');
require_once('flavor.inc');
require_once('flavorlist.inc');

/**
 * Nova is an abstraction layer for the OpenStack compute service
 *
 * Nova is used as a basis for several products, including Compute services
 * as well as Rackspace's Cloud Databases.
 */
abstract class Nova extends Service {
	private
	    $_namespaces=array(),
		$_url;

	/**
	 * Called when creating a new Compute service object
	 *
	 * _NOTE_ that the order of parameters for this is *different* from the
	 * parent Service class. This is because the earlier parameters are the
	 * ones that most typically change, whereas the later ones are not
	 * modified as often.
	 *
	 * @param \OpenCloud\Identity $conn - a connection object
	 * @param string $serviceRegion - identifies the region of this Compute
	 *      service
	 * @param string $urltype - identifies the URL type ("publicURL",
	 *      "privateURL")
	 * @param string $serviceName - identifies the name of the service in the
	 *      catalog
	 */
	public function __construct(OpenStack $conn,
	        $serviceType, $serviceName, $serviceRegion, $urltype) {
		$this->debug(_('initializing Nova...'));
		parent::__construct(
			$conn,
			$serviceType,
			$serviceName,
			$serviceRegion,
			$urltype
		);
		$this->_url = noslash(parent::Url());

		// load extension namespaces
		$this->load_namespaces();
	} // function __construct()

	/**
	 * returns the selected endpoint URL of this compute Service
	 *
	 * @param string $resource - a child resource. For example,
	 *      passing 'servers' would return .../servers. Should *not* be
	 *    prefixed with a slash (/).
	 * @param array $args (optional) an array of key-value pairs for query
	 *      strings to append to the URL
	 * @returns string - the requested URL
	 */
	public function Url($resource, $args=array()) {
	    $baseurl = $this->_url;
	    if ($resource != '')
	        $baseurl = noslash($baseurl).'/'.$resource;
	    if (!empty($args))
	        $baseurl .= '?'.$this->MakeQueryString($args);
		return $baseurl;
	}

	/**
	 * returns a flavor from the service
	 *
	 * This is a factory method and should generally be called instead of
	 * creating a Flavor object directly.
	 *
	 * @api
	 * @param string $id - if supplied, the Flavor identified by this is
	 *      retrieved
	 * @return Compute\Flavor object
	 */
	public function Flavor($id=NULL) {
	    return new Compute\Flavor($this, $id);
	} // flavor()

	/**
	 * Returns a list of flavors (class Flavor)
	 *
	 * This is a factory method and should generally be called instead of
	 * creating a FlavorList object directly.
	 *
	 * @api
	 * @param boolean $details - if TRUE (the default), returns full details.
	 *      Set to FALSE to retrieve minimal details and possibly improve
	 *      performance.
	 * @param array $filter - optional key/value pairs for creating query
	 *      strings
	 * @return Compute\FlavorList (or FALSE on an error)
	 */
	public function FlavorList($details=TRUE, $filter=array()) {
	    if ($details)
	        $url = $this->Url('flavors/detail', $filter);
	    else
	        $url = $this->Url('flavors', $filter);
	    $response = $this->Request($url);
	    $json = $response->HttpBody();
	    if (strlen($json) == 0)
	        return new Compute\FlavorList($this, array());
	    $obj = json_decode($json);
	    if ($this->CheckJsonError())
	    	return FALSE;
	    return new Compute\FlavorList($this, $obj->flavors);
	}

    /**
     * Gets a request from an HTTP source and ensures that the
     * content type is always application/json
     *
     * This is a simple subclass of the parent::Request() method that ensures
     * that all Compute requests use application/json as the Content-Type:
     *
     * @param string $url - the URL of the request
     * @param string $method - the HTTP method ("GET" by default)
     * @param array $headers - an associative array of headers to pass to
     *      the request
     * @param string $body - optional body for POST or PUT requests
     * @return \Rackspace\HttpResult object
     */
	public function Request($url, $method='GET', $headers=array(), $body=NULL) {
		$headers['Content-Type'] = 'application/json';
		return parent::Request($url, $method, $headers, $body);
	}

	/**
	 * Returns a list of supported namespaces
	 *
	 * @return array
	 */
	public function namespaces() {
	    return $this->_namespaces;
	}
	
	/********** PRIVATE METHODS **********/

	/**
	 * Loads the available namespaces from the /extensions resource
	 */
	private function load_namespaces() {
	    $ext = $this->Extensions();
	    foreach($ext as $obj)
	        $this->_namespaces[] = $obj->alias;
	}

} // class Compute