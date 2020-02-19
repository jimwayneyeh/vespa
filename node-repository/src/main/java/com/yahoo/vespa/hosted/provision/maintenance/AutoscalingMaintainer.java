// Copyright 2019 Oath Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.provision.maintenance;

import com.yahoo.config.provision.ApplicationId;
import com.yahoo.config.provision.ClusterSpec;
import com.yahoo.config.provision.NodeType;
import com.yahoo.vespa.hosted.provision.Node;
import com.yahoo.vespa.hosted.provision.NodeRepository;
import com.yahoo.vespa.hosted.provision.autoscale.Autoscaler;
import com.yahoo.vespa.hosted.provision.autoscale.ClusterResources;
import com.yahoo.vespa.hosted.provision.autoscale.NodeMetricsDb;

import java.time.Duration;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * Maintainer making automatic scaling decisions
 *
 * @author bratseth
 */
public class AutoscalingMaintainer extends Maintainer {

    private final Autoscaler autoscaler;

    public AutoscalingMaintainer(NodeRepository nodeRepository, NodeMetricsDb metricsDb, Duration interval) {
        super(nodeRepository, interval);
        this.autoscaler = new Autoscaler(metricsDb, nodeRepository);
    }

    @Override
    protected void maintain() {
        if ( ! nodeRepository().zone().environment().isProduction()) return;

        nodesByApplication().forEach((applicationId, nodes) -> autoscale(applicationId, nodes));
    }

    private void autoscale(ApplicationId applicationId, List<Node> applicationNodes) {
        nodesByCluster(applicationNodes).forEach((clusterSpec, clusterNodes) -> {
            Optional<ClusterResources> target = autoscaler.autoscale(applicationId, clusterSpec, clusterNodes);
            target.ifPresent(t -> log.info("Autoscale: Application " + applicationId + " cluster " + clusterSpec +
                                           " from " + applicationNodes.size() + " * " + applicationNodes.get(0).flavor().resources() +
                                           " to " + t.nodes() + " * " + t.resources()));
        });
    }

    private Map<ApplicationId, List<Node>> nodesByApplication() {
        return nodeRepository().list().nodeType(NodeType.tenant).state(Node.State.active).asList()
                               .stream().collect(Collectors.groupingBy(n -> n.allocation().get().owner()));
    }

    private Map<ClusterSpec, List<Node>> nodesByCluster(List<Node> applicationNodes) {
        return applicationNodes.stream().collect(Collectors.groupingBy(n -> n.allocation().get().membership().cluster()));
    }

}
